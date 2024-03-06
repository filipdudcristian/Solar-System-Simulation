#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

in vec3 fPosition;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
float shadow;

uniform float fogDensity;

uniform int lightMode;

uniform vec3 position;

vec3 pointAmbient;
vec3 pointDiffuse;
vec3 pointSpecular;

uniform vec3 pointLightColor;

float constant = 1.0f;
float linear = 0.054f;
float quadratic = 0.004f;//10f;

float ambientStrengthPoint = 0.05f;
float diffuseStrengthPoint = 0.8f;
float specularStrengthPoint = 1.0f;
float shininessPoint = 32.0f;

uniform mat4 model;
uniform mat4 view;


void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	
	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	//float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	
	return shadow;
}
void CalcPointLight()
{
    // important: transform world coordinates of the light to eye space coordinates
    //vec3 lightPos = (view * vec4(position, 1.0)).xyz; 
	vec3 normalEye = normalize(fNormal);
    
	vec3 lightDirN = normalize(position - fPosEye.xyz);
	vec3 viewDirN = normalize(-fPosEye.xyz);

    //compute ambient light
	pointAmbient = ambientStrength * pointLightColor;
    
	//compute diffuse light
	pointDiffuse = max(dot(normalEye, lightDirN), 0.0f) * pointLightColor; 
	
	//compute specular light
    vec3 reflection = normalize(reflect(-lightDirN, normalEye));
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0), shininess);

	pointSpecular = specularStrength * specCoeff * pointLightColor;

	//compute distance to light
	float dist = length(position - fPosition.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));


	pointAmbient *= att;
	pointDiffuse *= att;
	pointSpecular *= att;

	pointAmbient *= texture(diffuseTexture, fTexCoords).rgb; 
	pointDiffuse *= texture(diffuseTexture, fTexCoords).rgb; 
	pointSpecular *= texture(specularTexture, fTexCoords).rgb; 
} 



float computeFog()
{
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	ambient = vec3(0.0f);
	diffuse = vec3(0.0f);
	specular = vec3(0.0f);

	if(lightMode == 1)
	{
		computeLightComponents();
		ambient *= texture(diffuseTexture, fTexCoords).rgb;
		diffuse *= texture(diffuseTexture, fTexCoords).rgb;
		specular *= texture(specularTexture, fTexCoords).rgb;
	}
	else
	{
		CalcPointLight();

		ambient += pointAmbient;
		diffuse += pointDiffuse;
		specular += pointSpecular;
	}	

	vec3 color = min((ambient + diffuse) + specular , 1.0f);
	if(lightMode == 1)
	{
		shadow = computeShadow();
		color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	};
	
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.408f, 0.184f, 0.529f, 1.0f);
	fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
