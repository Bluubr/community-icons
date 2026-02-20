cmake_minimum_required(VERSION 3.25)

# 1. Project Name (Keep this short)
project(IconWorkshop)

# 2. Find and Link Geode
find_package(Geode REQUIRED)

# 3. Define your source files
# This adds main.cpp to the build
add_geode_mod(
    ${PROJECT_NAME}
    SOURCES 
        main.cpp
)

# 4. Resource Management
# This ensures your workshop-icon.png in the /resources folder
# is actually included in the final .geode file
setup_geode_mod(${PROJECT_NAME} RESOURCES resources/*.png)

# 5. Link Dependencies (Optional but good practice)
# If you use specific More Icons functions directly in C++, 
# you might need to add their headers here, but usually, 
# Geode handles the metadata linking via mod.json automatically.
