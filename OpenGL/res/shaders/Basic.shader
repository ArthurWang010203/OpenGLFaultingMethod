#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aNormal;

uniform mat4 u_MVP;

out vec3 Normal;
out vec3 FragPos;
void main() {
    gl_Position = u_MVP * vec4(position, 1.0);
    Normal = aNormal;
    FragPos = vec3(mat4(1.0) * vec4(position, 1.0));
};

#shader fragment
#version 330 core

in vec3 Normal;
in vec3 FragPos;
layout(location = 0) out vec4 color;
uniform vec4 u_Color;


void main() {
    vec3 norm = normalize(Normal);
    vec3 lightColor = (vec3(3.0f, 3.0f, 3.0f));
    vec3 lightPos = vec3(600.0f, 500.0f, 600.0f);
    vec3 lightDir = normalize(lightPos - FragPos);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 result = (ambient + diffuse) * u_Color.rgb;
    color = vec4(result,1.0);
    
};