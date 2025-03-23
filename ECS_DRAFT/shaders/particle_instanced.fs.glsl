#version 330

// From vertex shader
in vec2 texcoord;

// (x, y, z, a)
// if a > 0, xyza = rgba
// if a < 0, [x, y] = tex_u_range, z = alpha, -a = texture_id
in vec4 color_info;

// Texture data:
// Currently we are loading all data and switching to them based on texture id
// It is an ugly table mapping method, but should be enough for the project
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// Output color
layout(location = 0) out  vec4 color;


void main()
{
    if (color_info.a >= 0) {
        color = color_info;
    } else {
        float alpha = color_info.z;
        int texture_id = int(-color_info.a);

        switch (texture_id) {
            case 1:
                color = texture(texture1, texcoord);
                break;
            case 2:
                color = texture(texture2, texcoord);
                break;
            case 3:
                color = texture(texture3, texcoord);
                break;
            default:
                color = vec4(0.0, 0.0, 0.0, 1.0);
                break;
        }

        color.a = color.a * alpha;
    }
}
