<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:strip-space elements="q"/>
  <xsl:output method="text" encoding="UTF-8"/>
  
  <xsl:template match="p">
    <xsl:text>@(</xsl:text>
    <xsl:choose>
      <xsl:when test="contains(@corresp,'.')">
	<xsl:value-of select="substring-after(@corresp,'.')"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="@corresp"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="@n"/>
    <xsl:text>) </xsl:text>
    <xsl:apply-templates mode="p"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <!-- the following elements can occur in <p> in which context they
       are handled in p-mode -->
  <xsl:template mode="p" match="addSpan">
    <xsl:text>{</xsl:text>
  </xsl:template>
  <xsl:template mode="p" match="anchor">
    <xsl:text>} </xsl:text>
  </xsl:template>
  <xsl:template mode="p" match="foreign">
    <xsl:call-template name="foreign">
      <xsl:with-param name="p" select="true()"/>
    </xsl:call-template>
  </xsl:template>
  <xsl:template mode="p" match="gap">
    <xsl:value-of select="concat(' @gap{',@extent,'} ')"/>
  </xsl:template>
  <xsl:template mode="p" match="note">
    <xsl:variable name="targ" select="id(@target)"/>
    <xsl:value-of select="concat('@note',local-name($targ),'{')"/>
    <xsl:apply-templates mode="p"/>
    <xsl:text>}</xsl:text>
  </xsl:template>
  <xsl:template mode="p" match="q">
    <xsl:call-template name="q">
      <xsl:with-param name="p" select="true()"/>
    </xsl:call-template>
  </xsl:template>
  <xsl:template mode="p" match="ref">
    <xsl:apply-templates mode="p"/>
  </xsl:template>
  <xsl:template mode="p" match="w">
    <xsl:call-template name="w">
      <xsl:with-param name="p" select="true()"/>
    </xsl:call-template>
  </xsl:template>
  <xsl:template mode="p" match="xref">
    <xsl:call-template name="xref">
      <xsl:with-param name="p" select="true()"/>
    </xsl:call-template>
  </xsl:template>
  <xsl:template mode="p" match="text()">
    <xsl:value-of select="translate(.,'&#xa;',' ')"/>
  </xsl:template>

  <xsl:template mode="p" match="*">
    <xsl:message>tra.xsl: p-mode: unhandled element <xsl:value-of select="local-name(.)"/></xsl:message>
  </xsl:template>

  <!-- these are templates for elements that can occur both in <p> and
       elsewhere, for use in non-p-mode -->

  <xsl:template match="addSpan">
    <!--<xsl:text>{</xsl:text>-->
  </xsl:template>
  <xsl:template match="anchor">
    <!--<xsl:text>} </xsl:text>-->
  </xsl:template>
  <xsl:template match="foreign">
    <xsl:call-template name="foreign"/>
  </xsl:template>
  <xsl:template match="gap">
    <xsl:choose>
      <xsl:when test="count(preceding-sibling::*)=0">
	<xsl:text>&#xa;</xsl:text>
      </xsl:when>
      <xsl:when test="preceding-sibling::*[1][local-name()='q']">
	<xsl:text>&#xa;</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:value-of select="concat('$ (',@extent,')')"/>
  </xsl:template>
  <xsl:template match="note">
    <xsl:variable name="targ" select="id(@target)"/>
    <xsl:value-of select="'$ ('"/>
    <xsl:apply-templates/>
    <xsl:text>)&#xa;</xsl:text>
  </xsl:template>
  <xsl:template match="q">
    <xsl:call-template name="q"/>
  </xsl:template>
  <xsl:template match="ref">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="w">
    <xsl:call-template name="w"/>
  </xsl:template>
  <xsl:template match="xref">
    <xsl:call-template name="xref"/>
  </xsl:template>
  <!-- non-p-mode uses the default definition of match="text()"-->
  
  <!-- these are elements that only occur outside of <p> -->
  <xsl:template match="head">
    <xsl:value-of select="concat('@h1 ', text(), '&#xa;')"/>
  </xsl:template>
  <xsl:template match="lg">
    <xsl:text>#lg </xsl:text>
    <xsl:value-of select="@n"/>
    <xsl:text>&#x9;</xsl:text>
    <xsl:value-of select="@type"/>
    <xsl:text>&#xa;&#xa;</xsl:text>
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="trailer">
    <xsl:text>#trailer </xsl:text>
    <xsl:value-of select="@place"/>
    <xsl:text>&#x9;</xsl:text>
    <xsl:value-of select="@type"/>
    <xsl:text>&#xa;&#xa;</xsl:text>
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="altGrp"/>
  <xsl:template match="TEI.2|teiHeader|text|group|body|div1">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="fileDesc|revisionDesc|linkGrp"/>
  <xsl:template match="*">
    <xsl:message>tra.xsl: unhandled element <xsl:value-of select="local-name(.)"/></xsl:message>
  </xsl:template>

  <!-- function definitions for some elements that are treated the
       same in p-mode and non-p-mode -->
  <xsl:template name="foreign">
    <xsl:param name="p" select="false()"/>
    <xsl:value-of select="concat('@f',@lang,'{')"/>
    <xsl:choose>
      <xsl:when test="$p = true()">
	<xsl:apply-templates mode="p"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>}</xsl:text>
  </xsl:template>
  <xsl:template name="q">
    <xsl:param name="p" select="false()"/>
    <xsl:text>@q</xsl:text>
    <xsl:if test="string-length(@who)>0 or string-length(@toWhom)>0">
      <xsl:value-of select="concat('[',@who,',',@toWhom,']')"/>
    </xsl:if>
    <xsl:text>{“</xsl:text>
    <xsl:choose>
      <xsl:when test="$p = true()">
	<xsl:apply-templates mode="p"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>”}</xsl:text>
  </xsl:template>
  <xsl:template name="w">
    <xsl:param name="p" select="false()"/>
    <xsl:value-of select="concat('@w',@type,'{')"/>
    <xsl:choose>
      <xsl:when test="$p = true()">
	<xsl:apply-templates mode="p"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>}</xsl:text>
  </xsl:template>
  <xsl:template name="xref">
    <xsl:param name="p" select="false()"/>
    <xsl:value-of select="concat('@xref[',@doc,',',@from,']{')"/>
    <xsl:choose>
      <xsl:when test="$p = true()">
	<xsl:apply-templates mode="p"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>}</xsl:text>
  </xsl:template>

</xsl:transform>
