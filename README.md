# OpenGL_2
## Computer Graphics assignment in OpenGL

### Part 1: Common animation

For this part, we implemented the following:
* Rotation variable (rotationAmount) added.
* Model was made to rotate around X axis (later removed for other animations).
* Array of model objects added to mainview and implemented to render all of them.
* Designed model objects with their own buffers, transformation matrices, scale factors and animation/position definition vectors.
* New hammer and sickle mesh imported after triangulating the faces in Blender.
* Rotation and position definitions set for hammer and sickles.
* Rotation definitions in combination with position definitions used to make cats orbit center hammer and sickle.
* Scaling animation added to hammer and sickle.
* Perspective transform zoom/viewing angle modified by scaling bar.
* Perspective transform (eye location) modified by rotation controls. (move to origin, rotate, move to viewing distance).
* Cat texture modified.

### Part 2: Advanced vertex manipulation: water shader

For this part, we implemented the following:
* Created and modified a specific shader to transform the grid object into a waveform.
* Animated the waveform.
* Calculated the normals as they should be by derivating the waveform.
* Implemented multiple waves on top of each other.
* Used `smoothstep` and `mix` to simulate color variation based on wave height.
* Put the texture back on the wave to simulate waving flag.