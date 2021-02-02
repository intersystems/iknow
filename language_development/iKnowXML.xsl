<xsl:stylesheet xmlns:xsl = "http://www.w3.org/1999/XSL/Transform" version = "1.0" >
    <xsl:output method="html" version="1.0" encoding="UTF-8" indent="yes" />	
		  
	<xsl:key name="c_key" match="Concept" use="Index"/> <!-- collect the concepts -->
	<xsl:key name="r_key" match="Relation" use="Index"/> <!-- collect the relations -->
	<xsl:key name="crc_key" match="CRCPatterns" use="CRC"/> <!-- collect the CRC patterns -->
	<xsl:key name="cmatch_key" match="Concept/ExactMatch" use="OntoKey"/> <!-- collect the concept matches -->
	<xsl:key name="rmatch_key" match="Relation/ExactMatch" use="OntoKey"/> <!-- collect the relation matches -->
	
	<xsl:variable name="negation" select="'Negation'"/> <!-- link to ontoconcept in onto.csv file -->
		  
	<xsl:template match="/"> <!--	hoofdtemplate : de resultaten van de xslt worden teruggegeven in een html-pagina -->
	<html> 
	<head>
	<style type="text/css">
		.concept { font-weight:bold;background-color:azure; }
		.relation { text-decoration:underline;background-color:azure; }
		.nrelation { text-decoration:underline;background-color:red; }
		.nonrelevant { font-style:italic; }
		.pathrelevant { font-weight:bold;font-style:italic; }
		.modifier { color:blue; border: 2px solid blue; }
		.cluster { background-color:lightblue; }
		.evector { border: 1px solid; }
		<!-- currently expandable attributes -->
		.path_negation { font-weight:bold; background-color:red; }
		.negation { color:red; border: 2px solid red; }
		.path_sentpositive { color:purple; text-decoration:overline }
		.sentpositive { color:purple; text-decoration:overline }
		.path_sentnegative { color:brown; text-decoration:overline }
		.sentnegative { color:brown; text-decoration:overline }
		.path_time { font-weight:bold; background-color:green; }
		.time { color:green; border: 2px solid green; }	
		.path_frequency { font-weight:bold; background-color:purple; }
		.frequency { color:purple; border: 2px solid purple; }
		.path_duration { font-weight:bold; background-color:pink; }
		.duration { color:pink; border: 2px solid pink; }
		.path_measurement { font-weight:bold; background-color:grey; }
		.measurement { color:grey; border: 2px solid grey; }
		.path_certainty { font-weight:bold; background-color:yellow; }
		.certainty { border: 2px solid yellow; }
	</style>
	</head>
	<body>
	<!--
		tonen van de tekst met de correcte opmaak
		uit elk Sentence element halen we de Concept, Relation en NonRelevant deelelementen en geven deze door als parameter nodes aan de template handle_nodes
	-->
	<p>
	<xsl:for-each select="Content/Sentence">
		<br /><xsl:text>S(</xsl:text>
		<xsl:value-of select="position()" />
		<xsl:text>): </xsl:text>
		<xsl:variable name="nodes" select="Concept|Relation|NonRelevant|PathRelevant"/>
		<xsl:variable name="path_nodes" select="path"/>
		<xsl:variable name="entvector_nodes" select="entity_vector"/>
		<xsl:call-template name="handle_nodes">
			<xsl:with-param  name="nodes" select="$nodes"/>
		</xsl:call-template>
		<xsl:text>(Idx:)</xsl:text><br />
		<xsl:call-template name="handle_indexes">
			<xsl:with-param name="nodes" select="$nodes"/>
		</xsl:call-template>
		<xsl:if test="count($path_nodes) != 0">
		  <xsl:text>(Paths:)</xsl:text><br />
		  <xsl:call-template name="handle_paths">
			<xsl:with-param name="nodes" select="$path_nodes"/>
		  </xsl:call-template>
		</xsl:if>
		<xsl:if test="count($entvector_nodes) != 0">
		  <xsl:text>-Entity Vectors:</xsl:text><br />
		  <xsl:for-each select="$entvector_nodes">
		    <xsl:for-each select="ent">
			  <span class="evector"><xsl:value-of select="./text()" /></span>
			  <xsl:text>  </xsl:text>
			</xsl:for-each>
			<br />
		  </xsl:for-each>
		</xsl:if>
	</xsl:for-each>		
	</p>	
	</body>
	</html>	
	</xsl:template>
	
	<xsl:template name="handle_paths">
		<xsl:param name="nodes"/>
		<xsl:for-each select="path">
			<xsl:variable name="idxNode">
				<xsl:value-of select="position()" />
			</xsl:variable>
			<xsl:for-each select="value">
			  <xsl:value-of select="./text()" />
			</xsl:for-each>
			<xsl:for-each select="negation">
			  <br /><i><xsl:text>(negation) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_negation"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="sentiment_negative">
			  <br /><i><xsl:text>(sentiment_negative) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_sentnegative"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="sentiment_positive">
			  <br /><i><xsl:text>(sentiment_positive) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_sentpositive"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="time">
			  <br /><i><xsl:text>(date_time) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_time"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="measurement">
			  <br /><i><xsl:text>(measurement) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_measurement"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="frequency">
			  <br /><i><xsl:text>(frequency) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_frequency"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="duration">
			  <br /><i><xsl:text>(duration) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_duration"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>
			<xsl:for-each select="certainty">
			  <br /><i><xsl:text>(certainty) </xsl:text></i>
			  <xsl:for-each select="*">
			    <xsl:choose>
			      <xsl:when test="starts-with(name(),'no')">
			        <xsl:value-of select="./text()"/>
			      </xsl:when>
			      <xsl:otherwise>
				    <span class="path_certainty"><xsl:value-of select="./text()" /></span>
			      </xsl:otherwise>
				</xsl:choose>
				<xsl:text> </xsl:text>
			  </xsl:for-each>
			</xsl:for-each>

			<!--
			<xsl:value-of select="$nodes[number($idxNode)]/p_begin/text()" />
			<xsl:text> </xsl:text>
			<span class="nconcept"><xsl:value-of select="$nodes[number($idxNode)]/negation/text()" /></span>
			<xsl:text> </xsl:text>
			<xsl:value-of select="$nodes[number($idxNode)]/p_end/text()" />
			-->
			<br />
		</xsl:for-each>
	</xsl:template>
	<!--
	<span class="nconcept"><xsl:value-of select="$node/Literal/text()"/></span>
	<xsl:value-of select="$nodes[position()][self::path]" /><br />
	-->
	<xsl:template name="handle_indexes">
		<xsl:param name="nodes"/>
		<!-- <xsl:text>Handle_Indexes</xsl:text> -->
		<xsl:if test="count($nodes) != 0">
			<!-- <xsl:value-of select="name()" />
			<br />-->
			<!-- <xsl:for-each select="child::Concept">
				<xsl:value-of select="name()" />
			</xsl:for-each> -->
			<xsl:for-each select="*">
				<xsl:for-each select="Index">
				    <!-- <xsl:value-of select="name()"/> -->
				    <xsl:choose>
					  <xsl:when test="starts-with(name(),'negat')">
					      <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
					  </xsl:when>
					  <xsl:when test="starts-with(name(),'time')">
					      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
					  </xsl:when>
				      <xsl:when test="starts-with(name(),'mod')">
					      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
					  </xsl:when>
					  <xsl:otherwise>
						<span class="cluster"><xsl:value-of select="./text()"/></span>
					  </xsl:otherwise>
				    </xsl:choose>
					<xsl:text> </xsl:text>
				</xsl:for-each>
				<!-- <xsl:value-of select="./Index/text()"/>-->
				<xsl:text> </xsl:text>
			</xsl:for-each>
			<br />
		</xsl:if>	<!-- zero nodes -->
	</xsl:template>
	
	<!--	template handle_nodes

	de template die de tekst van een lijst van nodes (hier: Concept, Relation, NonRelevant, ... deelelementen van een Sentence element) met de gepaste opmaak zal teruggegeven
	de template voorziet alleeen het eerste element van de gepaste opmaak
	de overige elementen worden voorzien van opmaak door de recursieve template t_nodes

	parameter
	nodes		een lijst van Concept, Relation, NonRelevant, ... deelelementen van een Sentence element
-->
	<xsl:template name="handle_nodes">
		<xsl:param name="nodes"/>				
		<xsl:if test="count($nodes) != 0">
		
			<xsl:for-each select="Concept|Relation|NonRelevant|PathRelevant">
				<xsl:variable name="idxNode">
					<xsl:value-of select="position()" />
				</xsl:variable>
			  
			  <xsl:choose>
			    <xsl:when test="starts-with(name($nodes[number($idxNode)]),'Concept')">
					<xsl:call-template name="render_concept">	
					  <xsl:with-param name="type" select="'concept'"/>
					</xsl:call-template>
				</xsl:when>
			    <xsl:when test="starts-with(name($nodes[number($idxNode)]),'Relation')">
					<xsl:text>_</xsl:text>
					<xsl:call-template name="render_relation"></xsl:call-template>
					<xsl:text>_</xsl:text>
				</xsl:when>
			    <xsl:when test="starts-with(name($nodes[number($idxNode)]),'PathRelevant')">
					<xsl:text> </xsl:text>
					<!-- <span class="pathrelevant"><xsl:value-of select="./Literal/reg/text()"/></span> -->
					<xsl:call-template name="render_pathrelevant"></xsl:call-template>
					<xsl:text> </xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<!-- <span class="nonrelevant"><xsl:value-of select="./Literal/text()"/></span><xsl:text> </xsl:text> -->
					<xsl:call-template name="render_nonrelevant"/>
				</xsl:otherwise>
			  </xsl:choose>
			</xsl:for-each>
			<br />
			
		</xsl:if>	<!-- zero nodes -->
	</xsl:template>
	
<!--	template t_nodes

	recursieve template een gepaste opmaak geeft aan een lijst van nodes (de Concept, Relation, NonRelevant, ... deelelementen van een Sentence, met uitzondering van het eerste deelelement dat
	afgehandeld wordt door de template handle_nodes)
	
	
	parameters
	nodes				een lijst van Concept, Relation, NonRelevant, ... deelelementen van een Sentence element (met uitzondering van het eerste deelelement van het Sentence element)
	
	rel_negation		controle parameter die de waarde '1' bevat als de voorgaande niet Concept deelementen uit ten minste 1 negatie bestaan, en anders gelijk is aan '0'	
	in_concept_seq		controle parameter die de waarde '1' bevat als we in een rij van Concepten zitten (eventueel met elkaar verbonden met relaties AND, OR of OF), en anders gelijk is aan '0'
					een rij van Concepten wordt maar verlaten bij eerste relatie verschillend van AND, OR of OF
	of_relation			controleparameter die de waarde '1' bevat als de voorgaande Relation als index OF heeft, en anders gelijk aan '0'
	and_or_relation		controleparameter die de waarde '1' bevat als de voorgaande Relation als index AND of OR heeft, en anders gelijk aan '0'
	relevant_concept		controleparemeter die de waarde '1' bevat als het voorgaande Concept in een rij van Concepten relevant is, en anders gelijk aan '0'					
-->
	<xsl:template name="t_nodes">	
	<xsl:param name="nodes"/>
	<xsl:param name="rel_negation"/> 	
	<xsl:param name="in_concept_seq"/>
	<xsl:param name="of_relation"/>
	<xsl:param name="and_or_relation"/>
	<xsl:param name="relevant_concept"/>
	
		<xsl:if test="count($nodes) != 0">		
		<xsl:choose>	
		<!--CONCEPT: inkleuring van een Concept node--> 
			<xsl:when test="$nodes[1][self::Concept]">	
			<!--INKLEURING-->								
				<xsl:variable name="result_concept_contains_negation">
					<xsl:call-template name="concept_contains_negation">
						<xsl:with-param name="node" select="$nodes[1]"/>
					</xsl:call-template>
				</xsl:variable>
				
				<xsl:choose>
				<!--zonder voorgaande relatie met negatie-->
					<xsl:when test="$rel_negation = '0'">
						<xsl:variable name="tmp_relevant_concept">
							<xsl:choose>												
								<xsl:when test="$of_relation = '1'">
									<xsl:value-of select="$relevant_concept"/>
								</xsl:when>	
								<xsl:when test="$in_concept_seq = '1'">
									<xsl:choose>
										<xsl:when test="$result_concept_contains_negation = '1'">
											<xsl:text>0</xsl:text>
										</xsl:when>
										<xsl:otherwise>											
											<xsl:value-of select="$relevant_concept"/>
										</xsl:otherwise>
									</xsl:choose>								
								</xsl:when>
								<xsl:otherwise>
									<xsl:value-of select="1 - number($result_concept_contains_negation)"/>								
								</xsl:otherwise>									
							</xsl:choose>						
						</xsl:variable>
					
						<xsl:choose>
							<xsl:when test="$tmp_relevant_concept = '1'">
								<xsl:value-of select="$nodes[1]/Literal" />
								<xsl:text> </xsl:text>
							</xsl:when>
							<xsl:otherwise>
								<i><span style="color:red;"><xsl:value-of select="$nodes[1]/Literal/text()"/></span></i><xsl:text> </xsl:text>
							</xsl:otherwise>
						</xsl:choose>
						
					<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->
						<xsl:call-template name="t_nodes">
							<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
							<xsl:with-param name="rel_negation" select="$rel_negation"/>					
							<xsl:with-param name="in_concept_seq" select="'1'"/>
							<xsl:with-param name="of_relation" select="'0'"/>
							<xsl:with-param name="and_or_relation" select="'0'"/>
							<xsl:with-param name="relevant_concept" select="$tmp_relevant_concept"/>																					
						</xsl:call-template>
					</xsl:when>
				<!--met voorgaande relatie met negatie-->	
					<xsl:otherwise>
						<xsl:variable name="tmp_relevant_concept">
							<xsl:choose>								
								<xsl:when test="$of_relation = '1'">
									<xsl:value-of select="$relevant_concept"/>
								</xsl:when>	
								<xsl:when test="$in_concept_seq = '1'">
									<xsl:choose>									
										<xsl:when test="$result_concept_contains_negation = '1'">
											<xsl:text>1</xsl:text>
										</xsl:when>
										<xsl:otherwise>
											<xsl:value-of select="$relevant_concept"/>
										</xsl:otherwise>
									</xsl:choose>																										
								</xsl:when>									
								<xsl:otherwise>
									<xsl:value-of select="$result_concept_contains_negation"/>								
								</xsl:otherwise>																	
							</xsl:choose>						
						</xsl:variable>
					
						<xsl:choose>
							<xsl:when test="$tmp_relevant_concept = '1'">
								<xsl:value-of select="$nodes[1]/Literal" />
								<xsl:text> </xsl:text>
							</xsl:when>
							<xsl:otherwise>
								<i><span style="color:red;"><xsl:value-of select="$nodes[1]/Literal/text()"/></span></i><xsl:text> </xsl:text>
							</xsl:otherwise>
						</xsl:choose>				
						
					<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->
						<xsl:call-template name="t_nodes">
							<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
							<xsl:with-param name="rel_negation" select="$rel_negation"/>					
							<xsl:with-param name="in_concept_seq" select="'1'"/>
							<xsl:with-param name="of_relation" select="'0'"/>
							<xsl:with-param name="and_or_relation" select="'0'"/>
							<xsl:with-param name="relevant_concept" select="$tmp_relevant_concept"/>																					
						</xsl:call-template>					
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>	
		<!--RELATION: inkleuring van een Relation node-->
			<xsl:when test="$nodes[1][self::Relation]">			
			<!--CONTROLE: nagaan of we in een rij van concepten zitten-->
			<xsl:choose>	
				<xsl:when test="$in_concept_seq = '1'">
					<!--CONTROLE: nagaan of de relatie als index AND of OR heeft-->
					<xsl:variable name="result_and_or_relation"><!-- force false -->
						<xsl:text>0</xsl:text>
					</xsl:variable>
					
					<xsl:choose>
					<!--RESULTAAT: de relatie heeft als index AND of OR-->
						<xsl:when test="$result_and_or_relation = '1'">
						<!--INKLEURING: geen speciale inkleuring-->
					
							<xsl:value-of select="$nodes[1]/Literal/text()"/>
							<xsl:text> </xsl:text>										
					
						<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->
							<xsl:call-template name="t_nodes">
								<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
								<xsl:with-param name="rel_negation" select="$rel_negation"/>
								<xsl:with-param name="in_concept_seq" select="'1'"/>
								<xsl:with-param name="of_relation" select="'0'"/>
								<xsl:with-param name="and_or_relation" select="'1'"/>
								<xsl:with-param name="relevant_concept" select="$relevant_concept"/>
							</xsl:call-template>										
						</xsl:when>
					<!--RESULTAAT: de relatie heeft niet als index AND of OR-->	
						
						<xsl:otherwise>					
					<!--CONTROLE: nagaan of de relatie als index OF heeft-->
							<xsl:variable name="result_of_relation"><!-- force false -->
								<xsl:text>0</xsl:text>
							</xsl:variable>
							
							<xsl:choose>
								<xsl:when test="$result_of_relation = '1'">
								<!--INKLEURING: geen speciale inkleuring-->
								
									<xsl:value-of select="$nodes[1]/Literal/text()"/>								
									<xsl:text> </xsl:text>																														
									
								<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->								
									<xsl:call-template name="t_nodes">
										<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
										<xsl:with-param name="rel_negation" select="$rel_negation"/>					
										<xsl:with-param name="in_concept_seq" select="'1'"/>
										<xsl:with-param name="of_relation" select="'1'"/>
										<xsl:with-param name="and_or_relation" select="'0'"/>
										<xsl:with-param name="relevant_concept" select="$relevant_concept"/>
									</xsl:call-template>																																
						
								</xsl:when>
								
								<xsl:otherwise>
								<!--RESULTAAT: de relatie heeft niet als index AND, OR of OF-->
								<!--CONTROLE: nagaan of de relatie een negatie bevat-->	
									<xsl:variable name="result_relation_contains_negation">
										<xsl:call-template name="relation_contains_negation">
											<xsl:with-param name="node" select="$nodes[1]"/>
										</xsl:call-template>
									</xsl:variable>

																									
								<!-- INKLEURING: een Relation node die een negatie bevat, wordt STEEDS rood gekleurd en onderstreeept-->
									<xsl:choose>
										<xsl:when test="$result_relation_contains_negation = '1'">
											<span style="color:red;"><u><xsl:value-of select="$nodes[1]/Literal/text()"/></u></span>	
										</xsl:when>
										<xsl:otherwise>
											<xsl:value-of select="$nodes[1]/Literal/text()"/>
										</xsl:otherwise>
									</xsl:choose>
									<xsl:text> </xsl:text>																											

								<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->								
									<xsl:call-template name="t_nodes">
										<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
										<xsl:with-param name="rel_negation" select="$result_relation_contains_negation"/>
										<xsl:with-param name="in_concept_seq" select="'0'"/>
										<xsl:with-param name="of_relation" select="'0'"/>
										<xsl:with-param name="and_or_relation" select="'0'"/>
										<xsl:with-param name="relevant_concept" select="'0'"/>										
									</xsl:call-template>																																
								</xsl:otherwise>
							</xsl:choose>																				
						</xsl:otherwise>														
					</xsl:choose>
				</xsl:when>
				<xsl:otherwise>
				
				<!--CONTROLE: nagaan of de relatie een negatie bevat-->	
					<xsl:variable name="result_relation_contains_negation">
						<xsl:call-template name="relation_contains_negation">
							<xsl:with-param name="node" select="$nodes[1]"/>
						</xsl:call-template>
					</xsl:variable>

				<!--bepaling van de waarde die voor de parameter rel_negation moet doorgegeven worden
				     bij het vinden van een Concept node zal uiteindelijk rel_negation '1' zijn, als het aantal voorafgaande relaties met negatie oneven is,
				    anders is dit '0'
				-->																			
					<xsl:variable name="tmp_rel_negation">
						<xsl:choose>
							<xsl:when test="$rel_negation = '1'">
								<xsl:value-of select="number($rel_negation) - number($result_relation_contains_negation)"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="$result_relation_contains_negation"/>
							</xsl:otherwise>	
						</xsl:choose>	
					</xsl:variable>				
																
				<!-- INKLEURING: een Relation node die een negatie bevat, wordt STEEDS rood gekleurd en onderstreeept-->
					<xsl:choose>
						<xsl:when test="$result_relation_contains_negation = '1'">
							<span style="color:red;"><u><xsl:value-of select="$nodes[1]/Literal/text()"/></u></span>	
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select="$nodes[1]/Literal/text()"/>
						</xsl:otherwise>
					</xsl:choose>
					<xsl:text> </xsl:text>																											

				<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->								
					<xsl:call-template name="t_nodes">
						<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
						<xsl:with-param name="rel_negation" select="$tmp_rel_negation"/>					
						<xsl:with-param name="in_concept_seq" select="'0'"/>
						<xsl:with-param name="of_relation" select="'0'"/>
						<xsl:with-param name="and_or_relation" select="'0'"/>
						<xsl:with-param name="relevant_concept" select="'0'"/>																
					</xsl:call-template>																																
				</xsl:otherwise>
			</xsl:choose>
			</xsl:when>
		<!--NONRELEVANT,...: inkleuring van een NonRelevant node, van andere nodes...-->	
			<xsl:otherwise>
			<!--INKLEURING: gewone weergave van de node-->
				<xsl:value-of select="$nodes[1]/Literal/text()"/><xsl:text> </xsl:text>
			<!--RECURSIE: recursieve oproep van t_nodes om de volgende nodes in te kleuren-->	
				<xsl:call-template name="t_nodes">
					<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
					<xsl:with-param name="rel_negation" select="$rel_negation"/>					
					<xsl:with-param name="in_concept_seq" select="$in_concept_seq"/>
					<xsl:with-param name="of_relation" select="$of_relation"/>
					<xsl:with-param name="and_or_relation" select="$and_or_relation"/>
					<xsl:with-param name="relevant_concept" select="$relevant_concept"/>										
				</xsl:call-template>																				
					
			</xsl:otherwise>
		</xsl:choose>				
		</xsl:if>			
	</xsl:template>
	
	<!--	template relation_contains_negation

	de template controleert of een Relation een negatie bevat
	eerst  wordt nagegaan af de relatie een ExactMatch deelement heeft waarvan het OntoConcept overeenkomt met de variabele negation
	 - als dit geval is, dan wordt de waarde '1' teruggegeven
	 - anders wordt de template contains_negation opgeroepen waarbij de PartialMatch deelelementen van de Relation worden opgegeven voor de parameter nodes
	   de template geeft waarde '1' terug als de Relation een negatie bevat, anders wordt de waarde '0' teruggegeven
	
	parameter
	node		de te controleren Relation
	-->
	<xsl:template name="relation_contains_negation">
	<xsl:param name="node"/>
		<xsl:choose>
		<xsl:when test="$node/ExactMatch">
			<xsl:choose>
			<xsl:when test="$node/ExactMatch/OntoConcept/text() = $negation">
				<xsl:text>1</xsl:text>
			</xsl:when>
			<xsl:otherwise><xsl:text>0</xsl:text></xsl:otherwise>
			</xsl:choose>
		</xsl:when>
		<xsl:otherwise>
			<xsl:call-template name="contains_negation">
				<xsl:with-param name="nodes" select="$node/PartialMatch"/>
			</xsl:call-template>	
		</xsl:otherwise>
		</xsl:choose>	
	</xsl:template>
	
<!--	template handle_concept

	de template zorgt ervoor dat een Concept element de juiste opmaak krijgt (begin tekstdeel, negatie, inkleuring)
	hierbij wordt rekening gehouden met volgende aspecten
	- een Concept dat het begin van een nieuw onderdeel van de tekst aangeeft, wordt voorafgegaan door een break <br/> en wordt vetjes weergegeven (met <b></b>)
	  hiertoe wordt de template t_exact opgeroepen met de Index van het Concept element als parameter key
	- als het Concept niet het begin van een nieuw tekstdeel aangeeft, dan gaan we na of het Concept een negatie bevat
	  hiertoe wordt de template concept_contains_negation opgeroepen met het Concept als parameter node
	  een Concept dat een negatie bevat, wordt rood gekleurd
	 - als het Concept geen negatie bevat, dan wordt met de template handle_concept_partial opgeroepen met het Concept als parameter node
	   deze laatste template zal het Concept met de gepaste inkleuring teruggeven
	   
	 parameter
	 term		het Concept dat voorzien moet worden van de juiste opmaak
	 position	positie van de term		
-->
	<xsl:template name="handle_concept">	
		<xsl:param name="term"/>
		<xsl:param name="position"/>
		
		<xsl:variable name="result_exact">0</xsl:variable> <!-- force false -->
		<xsl:choose>
		<xsl:when test="$result_exact = '1'">									
			<br/><b><xsl:value-of select="$term/Literal"/></b>
		</xsl:when>				
		<xsl:otherwise>
			<xsl:choose>	
				<xsl:when test="contains($term/Index,' ')">						
					<!--controle of de conceptcluster een partial match bevat waarvan het OntoConcept overeenkomst met de variable negation-->
					<xsl:variable name="result_concept_contains_negation">
					<xsl:call-template name="concept_contains_negation">
						<xsl:with-param name="node" select="$term"/>
					</xsl:call-template>
					</xsl:variable>
				
					<xsl:choose>
						<xsl:when test="$result_concept_contains_negation = '1'">
						    <span style="color:red;"><xsl:value-of select="$term/Literal"/></span>							
						</xsl:when>	
						<xsl:otherwise>										
							<xsl:value-of select="$term/Literal" />
							<xsl:text> </xsl:text>
						</xsl:otherwise>
					</xsl:choose>	
				</xsl:when>	
				<xsl:otherwise>
					<xsl:value-of select="$term/Literal"/>
				</xsl:otherwise>				
			</xsl:choose>	
		</xsl:otherwise>	
		</xsl:choose>
		<xsl:text> </xsl:text>		
	</xsl:template>

<xsl:template name="render_entity">
	<xsl:for-each select="./Literal/*">
	    <xsl:choose>
		  <xsl:when test="starts-with(name(),'negat')">
		      <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'time')">
		      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'mod')">
		      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:otherwise>
			<span class="cluster"><xsl:value-of select="./text()"/></span>
		  </xsl:otherwise>
	    </xsl:choose>
		<xsl:text> </xsl:text>
	</xsl:for-each>
</xsl:template>

<xsl:template name="render_concept">
<xsl:param name="type"/>
	<!-- <xsl:value-of select="$type"/> -->
	<span class="concept">
	<xsl:for-each select="./Literal/*">
	    <xsl:choose>
		  <xsl:when test="starts-with(name(),'negat')">
		      <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'time')">
		      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'mod')">
		      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'sentpos')">
		      <span class="sentpositive" title="sentiment_positive"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'sentneg')">
		      <span class="sentnegative" title="sentiment_negative"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'frequency')">
		      <span class="frequency" title="frequency"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'duration')">
		      <span class="duration" title="duration"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'measurement')">
		      <span class="measurement" title="measurement"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'certainty')">
			<xsl:variable name="level_of_certainty"><xsl:value-of select="@level"/></xsl:variable>
			<xsl:variable name="tit_of_cert">(c=<xsl:value-of select="$level_of_certainty"/>)</xsl:variable>

		      <span class="certainty" title="certainty"><xsl:value-of select="./text()"/><xsl:value-of select="$tit_of_cert"/></span>
		  </xsl:when>		  
		  <xsl:otherwise><xsl:value-of select="./text()"/></xsl:otherwise>
	    </xsl:choose>
		<xsl:text> </xsl:text>
	</xsl:for-each>
	</span>
</xsl:template>

<xsl:template name="render_relation">
	<span class="relation">
	<xsl:for-each select="./Literal/*">
	    <xsl:choose>
		  <xsl:when test="starts-with(name(),'negat')">
			  <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'time')">
		      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'mod')">
		      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'sentpos')">
		      <span class="sentpositive" title="sentiment_positive"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'sentneg')">
		      <span class="sentnegative" title="sentiment_negative"><xsl:value-of select="./text()"/></span>
		  </xsl:when>	
	      <xsl:when test="starts-with(name(),'frequency')">
		      <span class="frequency" title="frequency"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'duration')">
		      <span class="duration" title="duration"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'measurement')">
		      <span class="measurement" title="measurement"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'certainty')">
			<xsl:variable name="level_of_certainty"><xsl:value-of select="@level"/></xsl:variable>
			<xsl:variable name="tit_of_cert">(c=<xsl:value-of select="$level_of_certainty"/>)</xsl:variable>
			
		      <span class="certainty" title="certainty"><xsl:value-of select="./text()"/><xsl:value-of select="$tit_of_cert"/></span>
		  </xsl:when>
		  
		  <xsl:otherwise><xsl:value-of select="./text()"/></xsl:otherwise>
	    </xsl:choose>
		<xsl:text> </xsl:text>
	</xsl:for-each>
	</span>
</xsl:template>

<xsl:template name="render_nonrelevant">
	<span class="nonrelevant">
	<xsl:for-each select="./Literal/*">
	    <xsl:choose>
		  <xsl:when test="starts-with(name(),'negat')">
			  <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'time')">
		      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'mod')">
		      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'sentpos')">
		      <span class="sentpositive" title="sentiment_positive"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'sentneg')">
		      <span class="sentnegative" title="sentiment_negative"><xsl:value-of select="./text()"/></span>
		  </xsl:when>		  
		  <xsl:otherwise><xsl:value-of select="./text()"/></xsl:otherwise>
	    </xsl:choose>
		<xsl:text> </xsl:text>
	</xsl:for-each>
	</span>
</xsl:template>

<xsl:template name="render_pathrelevant">
	<span class="pathrelevant">
	<xsl:for-each select="./Literal/*">
	    <xsl:choose>
		  <xsl:when test="starts-with(name(),'negat')">
			  <span class="negation" title="negation"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'time')">
		      <span class="time" title="time"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'mod')">
		      <span class="modifier" title="modifier"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
		  <xsl:when test="starts-with(name(),'sentpos')">
		      <span class="sentpositive" title="sentiment_positive"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'sentneg')">
		      <span class="sentnegative" title="sentiment_negative"><xsl:value-of select="./text()"/></span>
		  </xsl:when>	
	      <xsl:when test="starts-with(name(),'frequency')">
		      <span class="frequency" title="frequency"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'duration')">
		      <span class="duration" title="duration"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'measurement')">
		      <span class="measurement" title="measurement"><xsl:value-of select="./text()"/></span>
		  </xsl:when>
	      <xsl:when test="starts-with(name(),'certainty')">
		      <span class="certainty" title="certainty"><xsl:value-of select="./text()"/></span>
		  </xsl:when>		  		  
		  <xsl:otherwise><xsl:value-of select="./text()"/></xsl:otherwise>
	    </xsl:choose>
		<xsl:text> </xsl:text>
	</xsl:for-each>
	</span>
</xsl:template>
	
	<!--	template concept_contains_negation

	de template controleert of een Concept een negatie bevat
	hiertoe wordt de template contains_negation opgeroepen waarbij de PartialMatch deelelementen van het Concept worden opgegeven voor de parameter nodes
	de template geeft waarde '1' terug als het Concept een negatie bevat, anders wordt de waarde '0' teruggegeven
	
	parameter
	node		het te controleren Concept
-->
<!--xsl:when test="$node[@ContainsNegation] eq '1'"-->

<xsl:template name="new_contains_negation">
<xsl:param name="node"/>
<xsl:choose>
	<xsl:when test="@ContainsNegation='1'">
		<xsl:text>1</xsl:text>
	</xsl:when>
	<xsl:otherwise>
		<xsl:text>0</xsl:text>
	</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template name="concept_contains_negation">
<xsl:param name="node"/>
<xsl:call-template name="contains_negation">
	<xsl:with-param name="nodes" select="$node/PartialMatch"/>
</xsl:call-template>
</xsl:template>

<!--	template contains_negation (hulptemplate voor onder meer concept_contains_negation)

	de template controleert of een opgegeven lijst van elementen een negatie bevat
	dit gaat als volgt: een element bevat een negatie als de tekst van het deelelement OntoConcept overeenkomt met de variabele negation
	als er een element met een negatie wordt gevonden, dan geeft de template de waarde '1' terug, anders wordt de waarde '0' teruggegeven

	parameter
	nodes		lijst van te controleren elementen
-->

<xsl:template name="contains_negation">
<xsl:param name="nodes"/>
<xsl:choose>
	<xsl:when test="count($nodes) != 0">
		<xsl:choose>	
			<xsl:when test="$nodes[1]/OntoConcept/text() = $negation">				
				<xsl:text>1</xsl:text>
			</xsl:when>
			<xsl:otherwise>				
				<xsl:call-template name="contains_negation">
					<xsl:with-param name="nodes" select="$nodes[position() > 1]"/>					
				</xsl:call-template>				
			</xsl:otherwise>			
		</xsl:choose>		
	</xsl:when>	
	<xsl:otherwise>
		<xsl:text>0</xsl:text>
	</xsl:otherwise>
</xsl:choose>
</xsl:template>


</xsl:stylesheet> 