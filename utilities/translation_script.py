#HOW TO USE THIS FROM THE COMMAND LINE
#Format your entry as:
#File_path_to_python.exe [SPACE] file_path_to_python_script [SPACE] (optional) -o preferred_filename [SPACE] API_key [SPACE] Untranslated_xml


from lxml import etree
import urllib.request
from urllib.parse import quote
import re
import sys
import getopt


#Used to generate the output name of the new xml file
def output_name(input_name):
    output_string = ''
    for i in range(0,len(input_name)-3):
        output_string += input_name[i]
    output_string += '.ts'
    return output_string


#Used in translate function to format the output translation to match the source text format in the xml file. There was a problem with google translate adding/removing spaces before/after &amp;, &gt;, and &lt;.
#Google translate was also changing the encoding(i.e. &lt; to u\003c)
def append_to_dict(phrase_to_find, text, dict_name, dict_entry):

    regex = re.compile(phrase_to_find)
    num_indexes = [m.start(0) for m in re.finditer(regex, text)]
    for index in num_indexes:
        dict_name[index] = dict_entry


def assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, iteration, constant, target_phrase):

    index_tracker.append(index_list_tr[iteration] + constant)
    temp_string = translated_text[index_tracker[iteration]:index_tracker[iteration+1]]
    temp_string = temp_string.replace(r'\u003e', '&gt;')
    temp_string = temp_string.replace(r'\u003c', '&lt;')

    #if the translation added a space before the target then chop it off
    if translated_text[index_list_tr[iteration]-1] == ' ' and source_text[index_list_sr[iteration]-1] != ' ':
        temp_string = temp_string.replace(' ' + target_phrase, target_phrase, 1)

    #should chop is method through which spaces are taken away from the end of previous targets
    if should_chop[0]:
        if temp_string[0] == ' ':
            temp_string = temp_string.replace(' ', '', 1)
        should_chop[0] = False

    #if the translation added a space after the target then chop through bool(should_chop[0]). Conversely if translation took away a space at the end then add it back.
    if len(translated_text)-1 >= index_list_tr[iteration]+constant and len(source_text)-1 >= index_list_sr[iteration]+constant:
        if translated_text[index_list_tr[iteration]+constant] == ' ' and source_text[index_list_sr[iteration]+constant] != ' ':
            should_chop[0] = True
        if translated_text[index_list_tr[iteration]+constant] != ' ' and source_text[index_list_sr[iteration]+constant] == ' ':
            temp_string += ' '

    #if the translation took away a space before the target then add it back
    if translated_text[index_list_tr[iteration]-1] != ' ' and source_text[index_list_sr[iteration]-1] == ' ':
        new_string = ''
        index = temp_string.index(target_phrase)
        for z in range(0,len(temp_string)):
            if z != index:
                new_string += temp_string[z]
            else:
                new_string += ' '
                new_string += temp_string[z]
        temp_string = new_string

    #add formatted string to partial strings list
    partial_strings.append(temp_string)


def space_handler(source_text, translated_text):

    # SECTION 1: INITIALIZING VARIABLES
    index_tracker = [0]
    partial_strings = []
    output_string = ''
    should_chop = [False]

    index_dict_sr = {}
    index_dict_tr = {}
    index_list_sr = []
    index_list_tr = []

    append_to_dict(r'&amp;', source_text, index_dict_sr, 'a')
    append_to_dict(r'&amp;', translated_text, index_dict_tr, 'a')
    append_to_dict(r'&gt;', source_text, index_dict_sr, 'ga')
    append_to_dict(r'&gt;', translated_text, index_dict_tr, 'ga')
    append_to_dict('\\\\u003e', translated_text, index_dict_tr, 'gu')
    append_to_dict(r'&lt;', source_text, index_dict_sr, 'la')
    append_to_dict(r'&lt;', translated_text, index_dict_tr, 'la')
    append_to_dict('\\\\u003c', translated_text, index_dict_tr, 'lu')




    #SECTION 3: GENERATE INDEX LISTS AND SORT. DICTS ARE FOR REFERENCE
    #put the dict keys in lists and sort
    for key in index_dict_tr:
        index_list_tr.append(key)
    for key in index_dict_sr:
        index_list_sr.append(key)
    index_list_tr.sort()
    index_list_sr.sort()




    #SECTION 4: DIVIDE TRANSLATION UP INTO PARTIAL STRINGS AND REPLACE PARTS OF THE STRING ACCORDINGLY
    for i in range(0,len(index_list_tr)+1):

        if i == len(index_list_tr):
            temp_string = translated_text[index_tracker[i]::]
            if should_chop[0]:
               if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
            partial_strings.append(temp_string)
            break

        if index_dict_tr[index_list_tr[i]] == 'a':
            assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, i, 5, '&amp;')

        if index_dict_tr[index_list_tr[i]] == 'ga':
            assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, i, 4, '&gt;')

        if index_dict_tr[index_list_tr[i]] == 'gu':
            assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, i, 6, '&gt;')

        if index_dict_tr[index_list_tr[i]] == 'la':
            assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, i, 4, '&lt;')

        if index_dict_tr[index_list_tr[i]] == 'lu':
            assemble_partial_string(should_chop, partial_strings, index_tracker, index_list_tr, index_list_sr, source_text, translated_text, i, 6, '&lt;')




    #SECTION 5: GENERATE OUTPUT
    for string in partial_strings:
        output_string += string

    return output_string






#This function is the one used in the translate.xslt file
def translate(context, phrase, lang = 'fr' ): # will have to get information from locale

    # unreserved_characters = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
    #                          'A','B','C','D','E','F','G','H','I','J','K','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    #                          '0','1','2','3','4','5','6','7','8','9','-','_','.','~']

    reserved_characters = [' ','!','*','\'','(',')',';',':','@','&','=','+','$',',','/','?','%','#','[',']']

    base_url = 'https://www.googleapis.com/language/translate/v2?key='
    API_key = args[0]#MANUALLY ENTER API KEY
    source_language = 'source=en'
    translation_language = 'target=' + lang
    formatted_phrase = ''

    for letter in phrase:
        if letter in reserved_characters:
            formatted_phrase += quote(letter,safe="")
        else:
            formatted_phrase += letter

    url = base_url + API_key + '&' + source_language + '&' + translation_language + '&q=' + formatted_phrase

    try:
        stream = urllib.request.urlopen(url)
    except:
        return ""

    #The json output of the request to google may contain more than one string that matches the regex. The correct one to use is always first. Other matches give irrelevant information.
    translation_regex = re.compile(r'"translatedText":\s"(.*)"', re.IGNORECASE)
    translated_text = []
    mo = ''
    for line in stream:
        line = line.decode('UTF-8')
        mo = translation_regex.findall(line)
        if len(mo) != 0:
            translated_text.append(mo[0])

    translated_string = translated_text[0]

    #Since values get unencoded when reading the source text in xml files, reencode the ones that are used in the space_handler() function
    source_text = phrase
    source_text = source_text.replace('&', '&amp;')
    source_text = source_text.replace('<', '&lt;')
    source_text = source_text.replace('>', '&gt;')
    print("Out of Google Translate:      " + phrase)
    print("Reformatted source_text:      " + source_text)
    print("Original translation:         " + translated_string)

    #reformat spaces in translated string to match those in the source text
    translated_string = space_handler(source_text, translated_string)
    print("Space handled translation:    " + translated_string)
    print()


    return translated_string

arg_info = getopt.getopt(sys.argv[1:], "o:")
args = arg_info[1]

ns = etree.FunctionNamespace("mynamespace")
ns['translate'] = translate

dom = etree.parse(args[1]) #MANUALLY: place the base.ts as the argument

xslt = etree.parse("translate.xslt") #MANUALLY: place the xslt stylesheet as the argument
transform = etree.XSLT(xslt)

newdom = transform(dom)


#if there is a filename specified in the command line
if len(arg_info[0]) > 0:
    filename = arg_info[0][0][1]
else:
    filename = output_name(args[1]) #MANUALLY: name the output file


output_xml = open(filename,'w+')
output_xml.write(str(newdom))
output_xml.close()



