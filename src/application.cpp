#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include "light.h"

#include <cmath>

Application* Application::instance = NULL;
Camera* Application::camera = nullptr;

float cam_speed = 10;
bool render_wireframe = false;

//-----LIGHT-----------
Light* light = NULL;

//-----TEXTURE---------
//Helmet
Texture* albedo_text_H = NULL;
Texture* metalness_text_H = NULL;
Texture* normal_text_H = NULL;
Texture* roughness_text_H = NULL;
Texture* emissive_text_H = NULL;
//Lantern
Texture* albedo_text_L = NULL;
Texture* metalness_text_L = NULL;
Texture* normal_text_L = NULL;
Texture* roughness_text_L = NULL;
//Sphere
Texture* albedo_text_S = NULL;
Texture* metalness_text_S = NULL;
Texture* normal_text_S = NULL;
Texture* roughness_text_S = NULL;

//LUT-texture
Texture* LUT_texture = NULL;
//NULL-texture
Texture* emissive_text_null = NULL; //sphere and lanter are no emissive

//-----SHADER---------
Shader* shaderPBR = NULL;
Shader* shadSB = NULL;

//-----MATERIAL---------
PBR_material* pbr_material = NULL;
SBMaterial* SBmaterial = NULL;

//----ENVIRONMENT AND SKYBOX--------
Texture* textSB_0 = NULL;
Texture* textSB_1 = NULL;
Texture* textSB_2 = NULL;
Texture* textSB_3 = NULL;
Texture* textSB_4 = NULL;
Texture* textSB_5 = NULL;
Texture* textureSB = NULL;

//-------ImGui--------
int figure = 0; //0 -> helmet, 1 -> lantern, 2 -> sphere
bool objectSelect = true; //FALSE = helmet ; TRUE = lanter or sphere
int numShader = 2; //0 = DL ; 1 = IBL ; 2 = IBL+DL(+emissive)

//------ Meshes---------
Mesh* mesh_helmet = NULL;
Mesh* mesh_lantern = NULL;
Mesh* mesh_sphere = NULL;
Mesh* SBmesh = NULL;

//scale lantern
int helmet = 0;
int lantern = 0;
int sphere = 0;
//----------------------


Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(-5.f, 1.5f, 10.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	//----------------HELMET---------------------
	SceneNode* node_helmet = new SceneNode("Scene node Figure");
	node_list.push_back(node_helmet);
	mesh_helmet = new Mesh();
	mesh_helmet = Mesh::Get("data/meshes/helmet.obj");
	node_helmet->mesh = mesh_helmet;
	
	//----------------LANTERN---------------------
	SceneNode* node_lantern = new SceneNode("Scene node Lantern");
	mesh_lantern = new Mesh();
	mesh_lantern = Mesh::Get("data/meshes/lantern.obj");

	//----------------SPHERE---------------------
	SceneNode* node_sphere = new SceneNode("Scene node Sphere");
	mesh_sphere = new Mesh();
	mesh_sphere = Mesh::Get("data/meshes/sphere.obj");

	//----------------SKYBOX---------------------
	SceneNode* SBnode = new SceneNode("SkyBox");
	node_list.push_back(SBnode);
	SBmesh = new Mesh();
	SBmesh->createCube();
	SBnode->mesh = SBmesh;

	shadSB = new Shader;
	shadSB = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");

	HDRE* hdreSB = HDRE::Get("data/environments/shanghai.hdre");
	textureSB = new Texture;
	textureSB->cubemapFromHDRE(hdreSB);


	SBMaterial* SBmaterial = new SBMaterial(shadSB, textureSB);
	SBnode->material = SBmaterial;
	SBnode->model.setScale(1000, 1000, 1000);

		//---------Levels-----------
		textSB_0 = new Texture;
		textSB_0->cubemapFromHDRE(hdreSB, 0);
		textSB_1 = new Texture;
		textSB_1->cubemapFromHDRE(hdreSB, 1);
		textSB_2 = new Texture;
		textSB_2->cubemapFromHDRE(hdreSB, 2);
		textSB_3 = new Texture;
		textSB_3->cubemapFromHDRE(hdreSB, 3);
		textSB_4 = new Texture;
		textSB_4->cubemapFromHDRE(hdreSB, 4);
		textSB_5 = new Texture;
		textSB_5->cubemapFromHDRE(hdreSB, 5);
		//-------------------------

	//----------------------------Set textures, shaders and materials----------------------------

	LUT_texture = new Texture();
	LUT_texture = Texture::Get("data/textures/brdfLUT.png");

	//NULL texture
	emissive_text_null = new Texture();

	//Light-init
	light = new Light();

	//-------------------------HELMET-------------------------------
	albedo_text_H = Texture::Get("data/models/helmet/albedo.png");
	metalness_text_H = Texture::Get("data/models/helmet/metalness.png");
	normal_text_H = Texture::Get("data/models/helmet/normal.png");
	roughness_text_H = Texture::Get("data/models/helmet/roughness.png");
	emissive_text_H = Texture::Get("data/models/helmet/emissive.png");

	//-------------------------Lantern-------------------------------
	albedo_text_L = Texture::Get("data/models/lantern/albedo.png");
	metalness_text_L = Texture::Get("data/models/lantern/metalness.png");
	normal_text_L = Texture::Get("data/models/lantern/normal.png");
	roughness_text_L = Texture::Get("data/models/lantern/roughness.png");

	//-------------------------SPHERE-------------------------------
	albedo_text_S = Texture::Get("data/textures/color.png");
	metalness_text_S = Texture::Get("data/textures/metalness.png");
	normal_text_S = Texture::Get("data/textures/normal.png");
	roughness_text_S = Texture::Get("data/textures/roughness.png");

	//------------------------SHADER-----------------------------------
	shaderPBR = Shader::Get("data/shaders/basic.vs", "data/shaders/PBR.fs");

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(0, 0, 0, 1);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	for (int i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
	
}

void Application::update(double seconds_elapsed)
{

	//------------OUR CODE---------------------
	//Disable the depth test before rendering it
	//glDisable(GL_DEPTH_TEST);
	//SBnode->render(camera);
	//glEnable(GL_DEPTH_TEST);
	//-------------------------------------------

	mouse_locked = false;
	float speed = seconds_elapsed * cam_speed; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 0.5f;
	
	//camera speed modifier
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift

	float pan_speed = speed * 0.5f;

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))	camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed); 
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))	camera->move(Vector3( 0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))	camera->move(Vector3( 1.0f, 0.0f,  0.0f) * speed); 
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f,  0.0f) * speed); 

	//--------------------------OUR CODE---------------------------
	//if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))	SBnode->mesh->box.center.set(0.0f, 0.0f, 1.0f * speed);
	//if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))	SBnode->mesh->box.center.set(0.0f, 0.0f, -1.0f*speed);
	//if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))	SBnode->mesh->box.center.set(1.0f*speed, 0.0f, 0.0f);
	//if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) SBnode->mesh->box.center.set(-1.0f*speed, 0.0f, 0.0f);
	//-------------------------------------------------------------

	
	if (figure == 0) {

		lantern = 0;
		sphere = 0;

		objectSelect = false;
		
		node_list[0]->mesh = mesh_helmet;
		pbr_material = new PBR_material(objectSelect, float(numShader), light, albedo_text_H, metalness_text_H, normal_text_H, roughness_text_H, emissive_text_H, shaderPBR, textSB_0, textSB_1, textSB_2, textSB_3, textSB_4, textSB_5, LUT_texture);
		node_list[0]->material = pbr_material;
		if (helmet == 0) {
			node_list[0]->model.setScale(1, 1, 1);
			helmet = 1;
		}
	}
	else if (figure == 1) {

		helmet = 0;
		sphere = 0;

		objectSelect = true;
		
		node_list[0]->mesh = mesh_lantern;
		pbr_material = new PBR_material(objectSelect, float(numShader), light, albedo_text_L, metalness_text_L, normal_text_L, roughness_text_L, emissive_text_null, shaderPBR, textSB_0, textSB_1, textSB_2, textSB_3, textSB_4, textSB_5, LUT_texture);
		node_list[0]->material = pbr_material;
		if (lantern == 0) {
			node_list[0]->model.setScale(0.04, 0.04, 0.04);
			lantern = 1;
		}
	}

	else if (figure == 2) {

		helmet = 0;
		lantern = 0;

		objectSelect = true;

		node_list[0]->mesh = mesh_sphere;
		pbr_material = new PBR_material(objectSelect, float(numShader), light, albedo_text_S, metalness_text_S, normal_text_S, roughness_text_S, emissive_text_null, shaderPBR, textSB_0, textSB_1, textSB_2, textSB_3, textSB_4, textSB_5, LUT_texture);
		node_list[0]->material = pbr_material;
		if (sphere == 0) {
			node_list[0]->model.setScale(1, 1, 1);
			sphere = 1;
		}
	}

	if (!HoveringImGui()) 
	{
		//move in first person view
		if (mouse_locked || Input::mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			mouse_locked = true;
			camera->rotate(-Input::mouse_delta.x * orbit_speed * 0.5, Vector3(0, 1, 0));
			Vector3 right = camera->getLocalVector(Vector3(1, 0, 0));
			camera->rotate(-Input::mouse_delta.y * orbit_speed * 0.5, right);
		}

		//orbit around center
		else if (Input::mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) //is left button pressed?
		{
			mouse_locked = true;
			camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
		}

		//camera panning
		else if(Input::mouse_state& SDL_BUTTON(SDL_BUTTON_MIDDLE)) 
		{
				mouse_locked = true;
				camera->move(Vector3(-Input::mouse_delta.x * pan_speed, 0.f, 0.f));
				camera->move(Vector3(0.f, Input::mouse_delta.y * pan_speed, 0.f));
		}
	}

	//move up or down the camera using Q and E keys
	if (Input::isKeyPressed(SDL_SCANCODE_Q) || Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_E) || Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);
	
	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	SDL_ShowCursor(!mouse_locked);
	ImGui::SetMouseCursor(mouse_locked ? ImGuiMouseCursor_None : ImGuiMouseCursor_Arrow);
}

void Application::renderInMenu()
{
	// Show and edit your global variables on the fly here
	
	if (ImGui::TreeNode("Light"))
	{
		ImGui::NewLine();
		//----------------CHANGE LIGHT PROPERTIES---------------------------
		ImGui::ColorEdit3("Color", (float*)&light->color); // Edit 3 floats representing a color
		ImGui::NewLine();
		ImGui::DragFloat("Intensity", (float*)&light->intensity, 0.5f, 0.0f, 1000.0f);
		ImGui::NewLine();
		ImGui::Text("Position");
		ImGui::DragFloat3("Position", (float*)&light->position, 0.1f);
		ImGui::NewLine();
		ImGui::SliderFloat("X", (float*)&light->position.x, -50.0f, 50.0f, "%.4f");
		ImGui::SliderFloat("Y", (float*)&light->position.y, -250.0f, 250.0f, "%.4f");
		ImGui::SliderFloat("Z", (float*)&light->position.z, -250.0f, 250.0f, "%.4f");

		ImGui::NewLine();
		ImGui::TreePop();

	}

	if (ImGui::TreeNode("Figures"))
	{
		ImGui::NewLine();
		//--------------MENU WITH 3 FIGURES---------------------
		ImGui::RadioButton("Helmet", &figure, 0); ImGui::SameLine();
		ImGui::RadioButton("Latern", &figure, 1); ImGui::SameLine();
		ImGui::RadioButton("Sphere", &figure, 2);
		ImGui::NewLine();
		ImGui::TreePop();
	}


	if (ImGui::TreeNode("Shaders"))
	{
		ImGui::NewLine();
		//--------------MENU WITH 3 SHADERS---------------------
		ImGui::RadioButton("Direct Light", &numShader, 0); ImGui::SameLine();
		ImGui::RadioButton("IBL", &numShader, 1); ImGui::SameLine();
		ImGui::RadioButton("IBL + Direct Light", &numShader, 2);
		ImGui::NewLine();
		ImGui::TreePop();
	}
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F2: render_wireframe = !render_wireframe; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event){}
void Application::onGamepadButtonDown(SDL_JoyButtonEvent event){}
void Application::onGamepadButtonUp(SDL_JoyButtonEvent event){}
void Application::onMouseButtonDown( SDL_MouseButtonEvent event ){}
void Application::onMouseButtonUp(SDL_MouseButtonEvent event){}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	bool mouse_blocked = false;

	ImGuiIO& io = ImGui::GetIO();
	if (!mouse_locked)
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
		}
	mouse_blocked = ImGui::IsAnyWindowHovered();

	if (!mouse_blocked && event.y)
	{
		if (mouse_locked)
			cam_speed *= 1 + (event.y * 0.1);
		else
			camera->changeDistance(event.y * 0.5);
	}
}

void Application::onResize(int width, int height)
{
  std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

