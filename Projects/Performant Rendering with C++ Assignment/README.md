# Performant Rendering Assignment 
### Project Overview
This work demonstrates a critical understanding of component-based game engine architectures, including their design, implementation, and practical application. It reflects the ability to evaluate rendering techniques and make informed decisions about performance and memory usage within real-time systems. Through analysis and synthesis of different rendering approaches, this project highlights a considered and efficient approach to graphical problem-solving. <br><br>Additionally, it showcases the selection and application of appropriate methodologies, workflows, and toolchains tailored to the demands of performance-critical systems.

---
### What I Learned
- How to profile a program using tools like Tracy, RenderDoc, and Nsight.
- Reducing redundant binds using static variables and restructuring existing code.
- Using the [Meshoptimiser Library](https://github.com/zeux/meshoptimizer.git) and LODs to reduce draw calls.
- Programming with memory alignment in mind.
- CPU-side frustum culling.
---
### Running and Using The Application
Upon downloading the project, run the ```runCMake.bat``` to access the build folder and open ```AsteroidBelt.sln```. Run the program inside Visual Studio and use the controls listed below:
- **Movement:** W, S, A, D
- **Acceleration:** Arrow Keys
---
### Additional Information
- The project uses a renderer and demo written by the lecturer of this and the previous module. 
- As apart of the assignment, we were expected to profile and evaluate performance of the program after each optimisation was implemented. Those reports are inside the "Reports" folder.