// simple toon fragment shader
// www.lighthouse3d.com

vec4 toonify(in float intensity);

varying vec3 normal, lightDir;

void main() {
	float intensity;
	vec3 n;
	vec4 color;

	n = normalize(normal);
	intensity = max(dot(lightDir, n), 0.0); 	
	gl_FragColor = toonify(intensity);
}
