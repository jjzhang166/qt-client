/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "qbase64encode.h"

#include <qstring.h>
#include <qiodevice.h>
#include <qtextstream.h>
#include <qbuffer.h>

static const char _base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char getValue(char c) {
    int i = 0;
    while(_base64Table[i] != '\0') {
        if(_base64Table[i] == c) return (char)i;
        i++;
    }
    return (char)-1;
}

QString QBase64Encode(QIODevice & iod) {
    QString value;  // the string that holds the final encoded values
    int packet = 0; // there are 19 packets per line (packet=3 bytes in)

    char _in[3];
    unsigned char in[3];
    char out[4];
    Q_LONG didRead = 0;

    while(!iod.atEnd()) {
        // set the values to 0 in case we don't read all 3
        _in[0] = _in[1] = _in[2] = (char)0;

        // read in up to the next 3 chars
        didRead = iod.readBlock(&_in[0], 3);

        in[0] = (unsigned char)_in[0];
        in[1] = (unsigned char)_in[1];
        in[2] = (unsigned char)_in[2];

        if(didRead > 0) {
            // determine the encoded chars
            out[0] = _base64Table[ in[0] >> 2 ];
            out[1] = _base64Table[ ((in[0] & 0x03) << 4) | (in[1] >> 4) ];
            out[2] = _base64Table[ ((in[1] & 0x0F) << 2) | (in[2] >> 6) ];
            out[3] = _base64Table[ in[2] & 0x3F ];

            // modify the chars if we were short
            if(didRead < 3) {
                out[3] = '=';
                if(didRead < 2) {
                    out[2] = '=';
                }
            }

            // place the final chars in the value
            value += out[0];
            value += out[1];
            value += out[2];
            value += out[3];
        }


        if(++packet >= 19) {
            packet = 0;    // reset the packet count
            value += '\n'; // add a newline to the value
        }
    }
    value += '\n'; // throw one last newline onto the end

    return value;
}

QByteArray QBase64Decode(const QString & _source) {
    QString source = _source;
    QByteArray value;

    // empty string -- nothing to do
    if(source.isEmpty()) return value;

    QTextStream in(&source, QIODevice::ReadOnly);

    QBuffer buf(&value);
    buf.open(QIODevice::WriteOnly);

    char a[4], b[4], o[3];
    int n = 0;
    int p = 0; // current position in string
    int l = source.length(); // length of string
    char c = 0;
    while (p < l) {
        // read in the next 4 valid bytes
        n = 0;
        while(n < 4 && p < l) {
            c = ((QChar)source.at(p++)).toAscii();
            a[n] = c;
            b[n] = (c == '=' ? 0 : getValue(c));
            if(b[n] != -1) {
                n++;
            }
        }

        if(n < 4 && n != 0) {
            // whoops we have a mismatch in the number of bytes we need
        }

        // convert from base64 to binary
        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];

        // determine how many bytes we should be reading
        if(a[2] == '=')      n = 1;
        else if(a[3] == '=') n = 2;
        else                 n = 3;

        if(n != buf.writeBlock(&o[0], n)) {
            // eek we didn't write the number of block we were supposed to
        }

        if(n < 3) break; // we've reached the end of the data we have to read so just stop
    }

    return value;
}

