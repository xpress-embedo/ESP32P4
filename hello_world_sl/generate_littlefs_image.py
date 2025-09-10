import os
from littlefs import LittleFS

# Configuration
source_folder = "assets"
output_image = "assets_lfs.img"
block_size = 4096
block_count = 512  # 512 blocks * 4096 = 2MB

# Create LittleFS image
fs = LittleFS(block_size=block_size, block_count=block_count)

# Populate filesystem
for filename in os.listdir(source_folder):
    path = os.path.join(source_folder, filename)
    with open(path, "rb") as f:
        data = f.read()
    with fs.open(filename, "wb") as lfs_file:
        lfs_file.write(data)

# Export image
with open(output_image, "wb") as img_file:
    img_file.write(fs.context.buffer)

print(f"LittleFS image created: {output_image}")
