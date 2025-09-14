import json
from PIL import Image

def extract_platforms(path, width, height, cols, color, y_to_check, out_path):
    img = Image.open(path)
  #  img.show()
    spawn_data = {}
    prev_segments = []

    for frame_num in range(cols):
        left = frame_num * width
        frame = img.crop((left, 0, left + width, img.height))
   #     frame.show()

        all_segments_detected = []
        filtered_segments = []
        current_segment = None

        # scan along the y coordinate
        for x in range(frame.width):
            r,g,b,a = frame.getpixel((x, y_to_check))
            pixel = (r,g,b)
            print(pixel)
            # start a segment if we find the pixel color of a platform
            if pixel == color:
                if current_segment is None:
                    current_segment = [x,x]
                else:
                    current_segment[1] = x
            else:
                # if we find the end, mark the midpoint
                if current_segment:
                    mid_x = (current_segment[0] + current_segment[1]) // 2
                    all_segments_detected.append(mid_x)
                    if (mid_x not in prev_segments):
                        filtered_segments.append(mid_x)
                    current_segment = None


        if current_segment:
            mid_x = (current_segment[0] + current_segment[1]) // 2
            all_segments_detected.append(mid_x)
            if (mid_x not in prev_segments):
                filtered_segments.append(mid_x)

        spawn_data[frame_num] = filtered_segments
        prev_segments = all_segments_detected

    with open(out_path, 'w') as f:
        json.dump(spawn_data, f, indent=2)
    print(f"Spawn data saved to {out_path}")


if __name__ == "__main__":
    extract_platforms('C:\\Users\d\CPSC427\\team-03\ECS_DRAFT\data\\textures\\rolling_thing_full.png',
        800, 400, 66, (89, 76, 42), 70,
                      'C:\\Users\d\CPSC427\\team-03\ECS_DRAFT\data\\rolling_thing_platforms.json'
    )





