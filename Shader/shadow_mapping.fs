#version 460 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec4 FragPosViewSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform sampler2D shadowMap2;
uniform sampler2DArray shadowMapArray;

uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float lightRadius;
uniform float CSMPlaneDistances[16];
uniform int CSMAreaNum; 

layout (std140,binding = 0) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

layout (std140,binding = 1) uniform ShadowTypeUBO
{
    int ShadowMapType;
}; 

 

//普通的shadow Map
float ShadowCalculation(vec4 fragPosLightSpace)
{
    float shadow=0;
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias =max(0.015 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 转换到 [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // 采样ShadowMap中的深度
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 获取当前着色点的深度
    float currentDepth = projCoords.z;
    // PCF
    for(int i=-5;i<=5;++i)
    {
        for(int j=-5;j<=5;j++)
        {
            //  采样周围点在ShadowMap中的深度
            float closestDepth = texture(shadowMap, projCoords.xy+vec2(i,j)*texelSize).r; 
            shadow += currentDepth-bias > closestDepth  ? 1.0 : 0.0;
        }
    }

    

    return shadow/100;
}

//计算平均遮挡距离
float Calculate_Avg_Dblockreceiver(vec2 projCoords_xy , int AvgTextureSize,int layer)
{
    vec2 texelSize = ShadowMapType==1 ? 1.0 /vec2(textureSize(shadowMapArray, 0)) :  1.0 / textureSize(shadowMap, 0);
    
    float result=0.0f;
    for(int i=-AvgTextureSize;i<=AvgTextureSize;++i)
    {
        for(int j=-AvgTextureSize;j<=AvgTextureSize;j++)
        {

            result += ShadowMapType==1? texture(shadowMapArray, vec3( projCoords_xy+vec2(i,j)*texelSize,layer ) ).r  : texture(shadowMap, projCoords_xy+vec2(i,j)*texelSize).r; 
        }
    }
    return result/(AvgTextureSize*AvgTextureSize*2*2);

}

//PCSS
float PercentageCloserSoftShadowCalculation(vec4 fragPosLightSpace,int layer)
{
    float shadow=0;
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias =max(0.005 * (1.0 - dot(fs_in.Normal, lightDir)), 0.001);
    vec2 texelSize = ShadowMapType==1 ? 1.0 /vec2(textureSize(shadowMapArray, 0)) :  1.0 / textureSize(shadowMap, 0);
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 转换到 [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // 采样ShadowMap中的深度
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 获取当前着色点的深度
    float currentDepth = projCoords.z;
  
    //计算着色点与平均遮挡物的距离 dr
    float D_light_block=Calculate_Avg_Dblockreceiver(projCoords.xy,3,layer);
    float D_block_receiver= (currentDepth-D_light_block);
    // 检查当前点是否在阴影中
    if( D_light_block<0.01f)
        return 0.0f;
    //利用平均遮挡物距离dr计算PCF用到的采样范围 Wsample
    float fliterArea=D_block_receiver/(D_light_block*fragPosLightSpace.w) *lightRadius;
    int fliterSingleX=int(fliterArea);
    int count=0;

    fliterSingleX = fliterSingleX > 15 ? 15 : fliterSingleX;
    fliterSingleX = fliterSingleX < 1 ? 5 : fliterSingleX;
    //计算PCF
    for(int i=-fliterSingleX;i<=fliterSingleX;++i)
    {
        count++;
        for(int j=-fliterSingleX;j<=fliterSingleX;j++)
        {
            //  采样周围点在ShadowMap中的深度
            float closestDepth = ShadowMapType==1? texture(shadowMapArray, vec3( projCoords.xy+vec2(i,j)*texelSize,layer ) ).r  : texture(shadowMap, projCoords.xy+vec2(i,j)*texelSize).r; 
            shadow += currentDepth-bias > closestDepth  ? 1.0 : 0.0;
        }
    }
    count = count > 0? count : 1;
    shadow = shadow/float(count*count);
    
    
    return shadow ;
}


float CSMSoftShadowCalculation(vec4 fragPosWorldSpace)
{
    return 0.2f;
    //计算需要的深度layer
    float ViewPosDepth=abs(fragPosWorldSpace.z);
    int layer=-1;
    for(int i=0;i<CSMAreaNum;i++)
    {
        if(ViewPosDepth<CSMPlaneDistances[i])
        {
            layer=i;
            break;
        }
    }
    layer = step(0,layer)*(CSMAreaNum-1);

    //转换到对应的layer LightSpace
    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fs_in.FragPos, 1.0f);
    //计算PCSS
    /*float shadow=0;
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias =max(0.015 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == CSMAreaNum)
    {
        bias *= 1 / (CSMPlaneDistances[CSMAreaNum] * biasModifier);
    }
    else
    {
        bias *= 1 / (CSMPlaneDistances[layer] * biasModifier);
    }
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0));

    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 转换到 [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // 获取当前着色点的深度
    float currentDepth = projCoords.z;
    // PCF
    for(int i=-5;i<=5;++i)
    {
        for(int j=-5;j<=5;j++)
        {
            //  采样周围点在ShadowMap中的深度
            float closestDepth = texture(shadowMapArray, vec3(projCoords.xy + vec2(i,j) * texelSize, layer)).r;
            shadow += currentDepth-bias > closestDepth  ? 1.0 : 0.0;
        }
    }

    

    return shadow/100;*/
    return PercentageCloserSoftShadowCalculation(fragPosLightSpace,layer)
}



void main()
{      
 
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow;
    if(ShadowMapType==1)
    {
        shadow =  CSMSoftShadowCalculation(fs_in.FragPosViewSpace);
    }
    else if(ShadowMapType==2)
    {
        shadow = PercentageCloserSoftShadowCalculation(fs_in.FragPosLightSpace,0);                      
    }
    else
    {
        shadow = ShadowCalculation(fs_in.FragPosLightSpace) ;                    
    }
    vec3 lighting = (ambient + (1.0-shadow) * (diffuse + specular)) * color;    
    FragColor = vec4(lighting, 1.0);
}



