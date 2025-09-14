#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;

// Output color
layout(location = 0) out  vec4 color;


uniform int tile_id;

void main()
{
    // tilesheet curently 7x7
	const int tiles_per_row = 7;
    const float tile_size = 1.0 / float(tiles_per_row); // each tile occupies 1/7th of the texture
    
    // compute column and row  based on tile_id
    int col = (tile_id - 1) % tiles_per_row;
    int row = (tile_id - 1) / tiles_per_row;

    vec2 tile_offset = vec2(float(col), float(row)) * tile_size;


    vec2 tile_uv = tile_offset + texcoord * tile_size;

    color = texture(sampler0, tile_uv);
}
