 <xsl:stylesheet version="1.0"
          xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
          xmlns:tr="mynamespace" exclude-result-prefixes="tr">
     <xsl:output method="xml" omit-xml-declaration="yes" encoding="utf-8"/>




<xsl:template match="/">
   <xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE TS &gt;</xsl:text>
   <xsl:text>&#xa;</xsl:text>
   <xsl:copy>
      <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="@*|node()">
   <xsl:copy>
      <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>



<!--Matches message nodes that have translation nodes whose types are unfinished. Then the valid child nodes of message are matched so that they appear in the order specified by the documentation.-->
<xsl:template match="context/message[translation[@type='unfinished']]">
   <xsl:copy>
      <xsl:apply-templates select="location" />
      <xsl:apply-templates select="source" />
      <xsl:apply-templates select="oldsource" />
      <xsl:apply-templates select="comment" />
      <xsl:apply-templates select="oldcomment" />
      <xsl:apply-templates select="extracomment" />
      <xsl:apply-templates select="translatorcomment" />
      <xsl:text>&#xa;      </xsl:text>
      <xsl:element name="translation">
           <xsl:attribute name="type">
                <xsl:value-of select="'unfinished'" />
           </xsl:attribute>
      <xsl:value-of disable-output-escaping="yes" select="(tr:translate(string(source)))"/>
      </xsl:element>
      <xsl:apply-templates select="userdata" />
       <xsl:text>&#xa;    </xsl:text>
   </xsl:copy>
</xsl:template>


<xsl:template match="source">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="location">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="oldsource">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="comment">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="oldcomment">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="extracomment">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="translatorcomment">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


<xsl:template match="userdata">
   <xsl:text>&#xa;      </xsl:text>
   <xsl:copy>
       <xsl:apply-templates select="@* | node()" />
   </xsl:copy>
</xsl:template>


</xsl:stylesheet>


