This is my C++ Shader Programming assignment. The brief for the assignment was to "Effectively communicate knowledge of a shader programming workflow and realise an interactive 3D scene which is lit with a recognised simulated lighting model (Blinn Phong)". Currently there is a performance issue with the point lights as their positions are being called every frame rather than when the program first starts. This is something I am aware of and am in the process of fixing in my own time. However, performance issues aside, my work on this assignment got me 80% which is something I am very pleased with.

Below you will find instructions for how to run and interact with the program:

- W, A, S, D : Move forward (W), Left (A), backwards (S), and right (D)
- Mouse Scroll Wheel - Zoom in
- G : Toggle directional light
- H : Toggle normal mapping
- J : Toggle point lights	// Currently off due to aforementioned performance issues
- K : Toggle spotlight

- To run the program, open the Application.sln in Microsoft Visual Studio and navigate and start the program by pressing F5