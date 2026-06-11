# Final Project
### Project Overview
This project focused on the development of a custom 2D renderer, building upon the design and prototype created in the first project deliverable. The renderer was implemented as a game-related software artefact capable of supporting key rendering features such as sprite drawing, texture management, transformations, and camera systems. Throughout development, modern software engineering practices and game development methodologies were applied to produce a functional, maintainable, and extensible rendering framework.

The completed renderer was tested and evaluated against its original objectives to assess functionality, performance, and overall effectiveness. A reflective report critically discussed the design decisions made throughout development, the technical challenges encountered, and the solutions implemented. The project concluded with a viva demonstration that showcased the renderer's features and provided an overview of the development journey, highlighting the application of research, problem-solving skills, and technical communication within a games development context.

---
### What I Learned 
- Utilisation of SSBOs for programmable vertex pulling.
- Appropriate use of bindless textures compared to texture slots.
- Reading JSON to generate tilemaps.
- Batch rendering with instances using **gl_VertexID** and **gl_InstanceID**.
- Managing time to achieve a finished product in a reasonable timeframe.
---
### Running and Using The Application
Upon dowloading the project, run the ```runCmake.bat``` and open the ```finalProject.sln```. Alternatively, you can download and unzip a prebuilt version of my project and open ```finalProject.exe```.
- **Movement:** A, Arrow Key Left, D, Arrow Key Right, SPACE.
- **Aditional Key Binds**: R - Reset to start

---
#### De Montfort Univerity - P2802744
# 2D Renderer with Demonstration TDD
## Contents
  - [Technical Goals](#technical-goals)
  - [Technical Risks](#technical-risks)
  - [Code Style Guidelines](#code-style-guidelines)
    - [Naming Rules](#naming-rules)
      - [Indentation](#indentation)
      - [Iterator Variables](#iterator-variables)
      - [Static and Member Variables](#static-and-member-variables)
      - [Whitespace](#whitespace)
      - [Comments](#comments)
    - [Variables](#variables)
    - [Conditionals](#conditionals)
    - [Classes and Structs](#classes-and-structs)
  - [External Libraries](#external-libraries)
  - [How Data Flows](#how-data-flows)
    - [CPU-Side](#cpu-side)
    - [GPU-Side](#gpu-side)
      - [Vertex Shader](#vertex-shader)
      - [Fragment Shader](#fragment-shader)
    - [Programmable Vertex Pulling and Bindless Textures](#programmable-vertex-pulling-and-bindless-textures)
      - [Programmable Vertex Pulling](#programmable-vertex-pulling)
      - [Bindless Textures](#bindless-textures)
  - [Code Orgainisation Overview (UML)](#code-orgainisation-overview-uml)
  - [Bibliography](#bibliography)
  ---
## Technical Goals
*The technical goals are a list of objectives. They define what is expected to achieve in relation to the code, and renderer*
- Implement a functional 2D renderer using SSBOs, programmable vertex pulling, and bindless textures.
- Create a 2D platformer as a demonstration for the renderer.
- Ensure application is well commented and documented.
## Technical Risks 
*there are also risks which must not be overlooked. In order to take the risks into account they are written down in the TDD.*
- CPU vertex buffer and handle buffer could become full.
- Performance may be negatively impacted by lots of entities on the screen.
## Code Style Guidelines
*Being one of the most important of the TDD, in this section are described the code conventions that will be applied during the development of the whole project, in order to have an organized and homogeneous code. The thoroughness of the code style guidelines depends on the programmer and their objectives.*<br>
Code Style Guidelines constructed to be in line with [C++ Core Guidelines](#bibliography) [1]
### Naming Rules
*Explains in which language the code must be, how variables and functions must be named, the use of signs like parentheses and whitespaces or the need of using comments.*
- Code written in C++.
- All variable names written in English.
- Names must not be ambiguous.
#### Indentation
```cpp
if (conditional) {
    //!< Do Something
}
```
#### Iterator Variables
- For iterative loops containing a number, use ***i***. For nested loops, use ***j***, and ***k***.
```cpp
for (int i = 0; i < num; i++) {
    //!< Do Something
}
```
- Foreach loops will be favoured where applicable.
```cpp
for (thing : things) {
  //!< Do Something
}
```
#### Static and Member Variables
- Static variables should be prefixed with ***_s***, and member variables should be prefixed with ***_m***.
```cpp
float m_variable;
static float s_variable;
```
#### Whitespace
- Use to improve readability. Add spaces betweem operators and operands, commas, and semicolons.
```cpp
a = (b + c) * d
```
``` cpp
while (true)
```
```cpp
aFunction(param1, param2, param3)
```
```cpp
for (int i = 0; i < num; i++)
```
#### Comments
*Comments should follow [Doxygen Commenting syntax](#bibliography) [2]*
- File names should be commented as such
```cpp
/** @file file.cpp */
```
- Same line comments look as followed
```cpp 
if (a = b) {
    c++ //!< Iterates c
}
```
- Comment blocks are done and annotated with
```cpp
/**
 * @brief
 * @details
 * @param
 */
```
### Variables
*Explains how to manage the variables, how to declare and initialize them and whether to declare them inside public, protected or private part when using classes or structs.*
- Will be defined as public when they are to be accessed by a class that the class it is apart of does not inherit from.
- Will be defined as private when they are to only be accessed inside its class.
- Will be defined as protected when they are to be accessed by members that the class inherits from.
### Conditionals
*Describes how to manage conditionals, whether to separate the if line from the functions inside the conditional or to put everything in the same line, when to use scopes {} or if it is preferred a conditional with operators or without them.*
Conditionals should be structured as:
```cpp
if (thing) {

}
```
Rather than:
``` cpp
if (thing == true)
```
### Classes and Structs 
*It is basically a convention of when to use classes and when structs and how to write the elements inside them.*
- Class names should always be capitalised.
- Use structs when class could be broken down to a data structure.
```cpp
Class Something {
public:
    Something()
    ~Something()
    void publicFunction()
    float publicVariable{ 0.f };
private:
    void privateFunction();
private:
    float privateVariable{ 0.f };
}
```
- Variables of classes and structs should be orgainised with alignment in mind.
## External Libraries 
This project uses the following external libraries:
- [glm](#bibliography) [3]
- [glfw](#bibliography) [4]
- [GLAD](#bibliography) [5]
- [tracy](#bibliography) [6]
- [ENTT](#bibliography) [7]
- [STB](#bibliography) [8]
- [ImGui](#bibliography) [9]
## How Data Flows
### CPU-Side
Before accumilating data from the scene, space is reserved in the CPU-side vertex and texture handle buffers and empty SSBOs are created. 
```cpp
/** Vertex data SSBO setup */
m_cpuVertBuffer.reserve(s_vertexCapacity);
glCreateBuffers(1, &m_vertSsboID);
glNamedBufferStorage(m_vertSsboID, sizeof(Vertex) * m_cpuVertBuffer.capacity(), nullptr, GL_DYNAMIC_STORAGE_BIT);
glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vertSsboID); //!< Bind VBO to SSBO binding point 0 for shader access
```
```cpp
m_textureHandles.reserve(s_textureCapacity);
glCreateBuffers(1, &m_texSsboID);
glNamedBufferStorage(m_texSsboID, sizeof(uint64_t) * m_textureHandles.capacity(), nullptr, GL_DYNAMIC_STORAGE_BIT);
glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_texSsboID); //!< Bind VBO to SSBO binding point 1 for shader access
```
Below is a representation of what the buffers would look like before being populated: 
| idx | Colour [16bytes] | Position [8 bytes] | UV [8 bytes] |
| --- | --- | --- | --- |
| **0** | *empty* | *empty* | *empty* |
| **1** | *empty* | *empty* | *empty* |
| **2** | *empty* | *empty* | *empty* |
| **3** | *empty* | *empty* | *empty* |
| **...** | *empty* | *empty* | *empty* |
| **8191** | *empty* | *empty* | *empty* |

| idx | Texture Handle [64 bytes] | 
| --- | --- |
| **0** | *empty* | 
| **1** | *empty* | 
| **2** | *empty* | 
| **3** | *empty* |
| **...** | *empty* |
| **4095** | *empty* |

In the scene, the vertex data and texture handles of render components are generated. With the buffers populated, this a representation of what the buffers now look like:
```cpp
glm::vec4 m_defaultColour{ glm::vec4(1.0) }; //!< Default colour for vertices
```
```cpp
glm::vec2 quadVertices[] = {
	{-0.25f, -0.3f},	//!< Bottom-left
	{0.25f, -0.3f},		//!< Bottom-right
	{-0.25f, 0.3f},		//!< Top-left
	{0.25f, 0.3f}		//!< Top-right
};
```
| idx | Colour [16bytes] | Position [8 bytes] | UV [8 bytes] |
| --- | --- | --- | --- |
| **0** | m_defaultColour | quadVertices | { 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 1.f }, { 0.f, 0.f } |
| **1** | *empty* | *empty* | *empty* |
| **2** | *empty* | *empty* | *empty* |
| **3** | *empty* | *empty* | *empty* |
| **...** | *empty* | *empty* | *empty* |
| **8191** | *empty* | *empty* | *empty* |

| idx | Texture Handle [64 bytes] | 
| --- | --- |
| **0** | quadHandle | 
| **1** | *empty* | 
| **2** | *empty* | 
| **3** | *empty* |
| **...** | *empty* |
| **4095** | *empty* |

Once the process has finished, an instanced draw call is issued to send the data to the GPU. This will occur in two situations: 
- All the data from the entities has been added to the buffers.
- The buffers have reached capacity and need to be flushed.
```cpp
void Renderer::flush() {

	glNamedBufferSubData(m_vertSsboID, 0, sizeof(Vertex) * m_cpuVertBuffer.size(), (const void*)m_cpuVertBuffer.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vertSsboID); //!< Bind VBO to SSBO binding point 0 for shader access

	glNamedBufferSubData(m_texSsboID, 0, sizeof(uint64_t) * m_textureHandles.size(), (const void*)m_textureHandles.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_texSsboID); //!< Bind VBO to SSBO binding point 1 for shader access

	/** Bind VAO */
	glBindVertexArray(m_vaoID);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_cpuVertBuffer.size() / 4); //!< Draw the quads using instanced rendering
}
```
### GPU-Side 
#### Vertex Shader
With the data now on the GPU, the vertex shader pulls through the per vertex data using programmable vertex pulling per instance. Much like on the CPU, the vertex data is stored in a struct. 
```cpp
// Struct for Vertex GPU-side
struct Vertex {
	vec4 colour;
	float position[2];
	float uv[2];
};
```
Inside the Vertex shader are functions to construct the position and UV vectors that will then be accessed via an index.
```cpp
vec2 getPosition(int idx) {
	return vec2(vertices[idx].position[0], vertices[idx].position[1]);
}
```
```cpp
vec2 getUV(int idx) {
	return vec2(vertices[idx].uv[0], vertices[idx].uv[1]);
}
```
```cpp
int idx = gl_VertexID + (gl_InstanceID * 4); // Each instance has 4 vertices
```
***Vertex SSBO***
|idx | Colour | Position | UV  |
| --- | --- | --- | --- |
| **0** | {1.f, 1.f, 1.f, 1.f} | -0.25 | 1.0 |
| **1** | *empty* | -0.3 | 1.0 |
| **2** | *empty* | 0.25 | 1.0 |
| **3** | *empty* | -0.3 | 0.0 |
| **4** | *empty* | -0.25 | 0.0 |
| **5** | *empty* | 0.3 | 1.0 |
| **6** | *empty* | 0.25 | 0.0 |
| **7** | *empty* | 0.3 | 0.0 |
| **...** | *empty* | *empty* | *empty* |
| **8095** | *empty* | *empty* | *empty* |

The colour, UVs, and Instance IDs are then passed to fragment shader for bindless textures.
```glsl
out vec2 aUV;
out vec4 aCol;
flat out int fsInstance; // Needed to select which texture 
```
#### Fragment Shader
The populated texture handle SSBO looks as followed: <br><br>
***Texture Handle SSBO***
| idx | TexHandle |
| --- | --- |
| **0** | 0 |
| **1** | 64 |
| **2** | *empty* |
| **...** | *empty* |
| **4095** | *empty* |

The texture handles are stored as Sampler2Ds which are equivalent to uint64_t.
```cpp
layout (std430, binding  = 1) readonly buffer handlesSSBO {
	sampler2D textureHandles[];
};
```
As I am using bindless textures, an extension needs to be included at the top of both my vertex and fragment shader. Below is the line that is required for bindless textures to be used:
```glsl
#extension GL_ARB_bindless_texture : require
```
***NOTE: I have a default white texture that enables coloured quads as the texture is simply multiplied by a colour. This also enables me to tint textures if that were something I wanted.***<br><br>
With that all set up correctly, its a case of colouring the fragments based on the texture and/or colour that is passed in. The coloured fragments are then rendererd to the screen.
### Programmable Vertex Pulling and Bindless Textures
*Programmable Vertex Pulling and Bindless Textures have their benefits and drawbacks over their alternatives.  This section goes through my reasoning for choosing them*
#### Programmable Vertex Pulling
The alternative to using programmable vertex pulling as SSBOs is to use VBOs. Both have their use cases but I found SSBOs more versatile and better suited for my use case. Specifically for a 2D renderer, using SSBOs and PVP made more sense for the following reasons:
| Feature | VAO/VBO | SSBO (Programmable Vertex Pulling) |
| --- | --- | --- |
| *Flexability* | Low - Strict layout | High - Custom struct handling |
| *Setup* | VAO/VBO management | Low (Buffer Binding) |
| *Memory* | Higher - Stores every vertex | Lower - Stores minimal data, calculates rest |
| *Performance* | High (Dedicated Hardware) | High (Modern GPUs) |
| *Best For* | Thousands of simple, static, <br>or dynamic sprites | Large-scale 2D, dense tilemaps, particles, <br>and post-processing |

- Can pack any data  structure into an SSBO and manually unpack it in GLSL.
- Enabes storing data per instance rather than per vertex using ***gl_InstanceID*** and ***gl_VertexID***.
- Batch rendering easier and more performant.

Though I consider it the better option, it does come with its down sides:
- SSBOs were introduced in GLSL Version #430 meaning more modern hardware is required to make use of it.
- Can be sometimes slower on older hardware or specific vendors.
#### Bindless Textures
Using bindless textures as oppose to using texture slots enables me to have as close to as many textures as I'd like as possible and means I don't have to manage texture slots in the instance I go above the default 32 slots. Given I will be working with tilemaps and spritesheets, I view that as a neccessity as I'd very quicky fill up the 32 texture slots. However, bindless textures aren't without their downsides:
- Unable to debug renderer with applications such as RenderDoc as bindless textures are not supported.
- Bindless textures have to be enabled via an extension in GLSL meaning they are not natively supported.
- Following on from the previous point, bindless textures may not be supported by specific vendors.
## Code Orgainisation Overview (UML)
*The UML offers a general outlook of the structure the code will have and the inheritance relation between its classes. UML diagrams show the functions and the variables included in each module. It is a guide used to see the whole picture of the code.*

![Figure 1][UML.png]
- UML diagram was created using [draw.io](#bibliography) [10]
- [Guide used to create UML](#bibliography) [11]
## Bibliography
*Citations are done using the APA 7 style.*
- [1] C++ Core Guidelines. (n.d.). Isocpp.github.io. https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- [2] Doxygen Manual: Documenting the code. (n.d.). Www.doxygen.nl. https://www.doxygen.nl/manual/docblocks.html
- [3] g-truc. (2025). GitHub - g-truc/glm: OpenGL Mathematics (GLM). GitHub.
https://github.com/g-truc/glm.git
- [4] glfw. (2024, February 23). GitHub - glfw/glfw: A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input. GitHub. https://github.com/glfw/glfw.git
- [5] SimonCouplandDMU. (2025, April 4). GitHub - SimonCouplandDMU/DMUGlad: Glad for use in DMU C++ modules. GitHub. https://github.com/SimonCouplandDMU/DMUGlad.git
- [6] wolfpld. (2025, December 11). GitHub - wolfpld/tracy: Frame profiler. GitHub. https://github.com/wolfpld/tracy.git
- [7] skypjack. (2025, November 20). GitHub - skypjack/entt: Gaming meets modern C++ - a fast and reliable entity component system (ECS) and much more. GitHub. https://github.com/skypjack/entt.git
- [8] nothings. (2025). GitHub - nothings/stb: stb single-file public domain libraries for C/C++. GitHub. https://github.com/nothings/stb.git
- [9] ocornut. (2019, December 6). ocornut/imgui. GitHub. https://github.com/ocornut/imgui
- [10] draw.io. (2025). Flowchart Maker & Online Diagram Software. App.diagrams.net. https://app.diagrams.net/
- [11] Visual Paradigm. (2024). UML Class Diagram Tutorial. Visual-Paradigm.com. https://www.visual-paradigm.com/guide/uml-unified-modeling-language/uml-class-diagram-tutorial/

[UML.png]: UML.png