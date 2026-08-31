<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text"/>
  <xsl:template match="addSpan|anchor">
    <xsl:copy-of select="concat(local-name(.),'&#x9;',@xml:id|@to,'&#xa;')"/>
  </xsl:template>
  <xsl:template match="text()"/>
</xsl:transform>
