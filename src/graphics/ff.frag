precision mediump float;

uniform bool useTexCoords;
// uniform bool useDiffuse;
uniform sampler2D tex;
uniform vec4 fogColor;
uniform int colorOp;
uniform vec4 envDiffuse;

varying mediump vec2 interpTexCoords;
varying lowp vec4 interpDiffuse;
varying mediump float fogCoeff;

const float alphaThreshold = 4.0 / 255.0;

#define OP_MODULATE 0
#define OP_ADD 1
#define OP_REPLACE 2

void main()
{
    vec4 fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 fragArg1 = fragColor;
    vec4 fragArg2 = fragColor;

    vec4 texColor = texture2D(tex, interpTexCoords);
    fragArg1 = mix(interpDiffuse, texColor, float(useTexCoords));

#ifndef NO_VERTEX_BUFFER
    fragArg2 = envDiffuse;
#else
    fragArg2 = interpDiffuse;
#endif

    vec4 modulate = fragArg1 * fragArg2;

    vec4 add;
    add.rgb = min(fragArg1.rgb + fragArg2.rgb, vec3(1.0));
    add.a   = fragArg1.a * fragArg2.a;

    vec4 replace = fragArg1;

    fragColor =
        modulate * float(colorOp == OP_MODULATE) +
        add      * float(colorOp == OP_ADD) +
        replace  * float(colorOp == OP_REPLACE);

#ifndef NO_FOG
    fragColor.rgb = mix(fogColor.rgb, fragColor.rgb, fogCoeff);
#endif

    fragColor.a = max(fragColor.a, alphaThreshold);

    gl_FragColor = fragColor;
}
