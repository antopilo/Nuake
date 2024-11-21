#pragma once
/*
Welcome to NuakeUI, 
first of all thank you for using this library.

How to use:
	1.	Include this header file
	2.	Create a CanvasParser object and parse an xml file using the Parse method.
		It returns a pointer to your canvas object.
	3.	Create an InputManager class that inherits from the InputManager.h interface.
	4.	Register the input manager on your canvas object returned by the Parse method of earlier with the SetInputManager method
	5.	Call the 3 follow methods in your main loop:
			1. canvas->ComputeLayout(myWindowSize);
			2. canvas->Tick();
			3. canvas->Draw();
	6.	You are now rendering your UI.

Contribute:
	If you wish to know more about the other features like DataModel binding, styling options,
	Fragments, and responsive layouts, you should look at the ReadMe of the repository or look at the 
	demo projects which goes more in depth.

	Original repository: https://github.com/antopilo/nuakeui
	Demo repository: https://github.com/antopilo/nuakeuidemo

In case you have any feature requests, or encounter issues with the project.
Please fill them on the issues page on the repository.

Thank you, 
antopilo

*/

#include "src/UI/Nodes/Button.h"
#include "src/UI/Renderer.h"

#include "Parsers/CanvasParser.h"

/* 
TODO:
	- FIX z-index mouse hover
	- FIX mouse events when window not focused
	- CSS Variables	
		- Color
		- Units
		- Make them global?
	- CSS background image
	- <div> Active state
	- <svg> Tag? with nano svg
	- Text overflow clipping not working
*/