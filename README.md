# single_file_gltf_importer
A single cpp file gltf importer that works with glm

Setup:
1. Make a new visual studio project and place these folders in the working directory(have [Place solution and project in the same directory DISABLED])
2. Go to C/C++ -> General -> Additional Include Directories and paste this in [$(SolutionDir)<projectName>\src;$(SolutionDir)<projectName>\external\include]
3. run the program and the importer with generate std::vectors of positions, indices and texCoords. Use them however you want.
