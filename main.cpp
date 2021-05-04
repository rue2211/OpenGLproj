//!Includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>

//!Function Prototypes
bool initGL(int argc, char** argv);
void initShader();
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void render2dText(std::string text, float r, float g, float b, float x, float y);
void initTexture(std::string filename, GLuint & textureID);

//Global Variables                                                                          
GLuint shaderProgramID;                   // Shader Program ID            
GLuint vertexPositionAttribute;           // Vertex Position Attribute Location
GLuint vertexNormalAttribute;           // Vertex Position Attribute Location
GLuint vertexTexcoordAttribute;           // Vertex Texcoord Attribute Location         
GLuint TextureMapUniformLocation;              // Texture Map Location      
GLuint texture_plane;                                   // OpenGL Texture    
GLuint texture_ground;
GLuint texture_sky;
Vector3f plane_position(0.0,0.0,0.0);                        
float t_global = 0.0;
                 
//Viewing                                                                                   
Matrix4x4 ModelViewMatrix;              // ModelView Matrix                                 
GLuint MVMatrixUniformLocation;         // ModelView Matrix Uniform                         
Matrix4x4 ProjectionMatrix;             // Projection Matrix                                
GLuint ProjectionUniformLocation;       // Projection Matrix Uniform Location               
                                                                                           
SphericalCameraManipulator cameraManip;

//Mesh                                                                                      
Mesh mesh_plane; 
Mesh mesh_ground; 
Mesh mesh_sky; 

//! Screen size
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//! Array of key states
bool keyStates[256];


//! Main Program Entry
int main(int argc, char** argv)
{	
	//init OpenGL
	if(!initGL(argc, argv))
		return -1;

    //Init Key States to false;    
    for(int i = 0 ; i < 256; i++)
        keyStates[i] = false;
    
    //TODO: Set up your shader program
    initShader();

	//Init Mesh Geometry                                                                          
        //mesh.initCube();                                                                            
    mesh_plane.loadOBJ("../models/plane1.obj");                                               
        initTexture("../models/plane1.bmp", texture_plane);

	mesh_ground.loadOBJ("../models/ground.obj");                                               
        initTexture("../models/grass.bmp", texture_ground);

	mesh_sky.loadOBJ("../models/cube2.obj");                                               
        initTexture("../models/sky1.bmp", texture_sky);

    //Init Camera Manipultor                                                                          
        //cameraManip.setPanTiltRadius(0.f,0.f,2.f);
        //cameraManip.setFocus(mesh_plane.getMeshCentroid());


    //Enter main loop
    glutMainLoop();

    //Clean-Up                                                                          
    glDeleteProgram(shaderProgramID);

    return 0;
}

//! Function to Initialise OpenGL
bool initGL(int argc, char** argv)
{
	//Init GLUT
    glutInit(&argc, argv);
    
	//Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//Set Window Size
    glutInitWindowSize(screenWidth, screenHeight);
    
    // Window Position
    glutInitWindowPosition(200, 200);

	//Create Window
    glutCreateWindow("Plane Assignment");
    
    // Init GLEW
	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}
	
	//Set Display function
    glutDisplayFunc(display);
	
	//Set Keyboard Interaction Functions
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp); 

	//Set Mouse Interaction Functions
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);
	
	//Enable texture mapping 
 	glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    //Start start timer function after 100 milliseconds
    glutTimerFunc(100,Timer, 0);

	return true;
}

// TODO
void initShader()
{
	//Create shader                                                                               
    shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");

    // Get a handle for our vertex position buffer                                                    
        vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	
		vertexNormalAttribute = glGetAttribLocation(shaderProgramID, "aVertexNormal");

    //!                                                                                               
        MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform");

        ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform");

	//initialise the global variables                                                             
        vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");

	TextureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");

	//LightPositionUniformLocation = glGetUniformLocation(shaderProgramID, "LightPosition_uniform");
        //AmbientUniformLocation = glGetUniformLocation(shaderProgramID, "Ambient_uniform");
        //SpecularUniformLocation = glGetUniformLocation(shaderProgramID, "Specular_uniform");
        //SpecularPowerUniformLocation = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");

}

void initTexture(std::string filename, GLuint & textureID)
{
	//Generate texture and make it the current texture
	 glGenTextures(1, &textureID);
	 glBindTexture(GL_TEXTURE_2D, textureID);
	
	//Set internal parameters of texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	//Get texture Data
	int width, height;
	char* data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//glBindTexture(GL_TEXTURE_2D, 0);	//!!!!!
	//Cleanup data as copied to GPU
	delete[] data;
}


//! Display Loop
void display(void)
{
    //Handle keys
    handleKeys();

	//Set Viewport
	glViewport(0,0,screenWidth, screenHeight);
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //TODO:Draw your scene
    
    	//Use shader
	glUseProgram(shaderProgramID);

	//Projection Matrix - Perspective Projection
    	ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);


 	//Set Projection Matrix
    	glUniformMatrix4fv(	
		ProjectionUniformLocation,  //Uniform location
		1,							//Number of Uniforms
		false,						//Transpose Matrix
		ProjectionMatrix.getPtr());	//Pointer to ModelViewMatrixValues

    ModelViewMatrix.toIdentity();

	// t_global++;

	//use Lookat function
	ModelViewMatrix.lookAt(
		Vector3f(0.01, 0, 3),
		Vector3f(0,0,0),
		Vector3f(0, 1,0)
	);
	
	
   	// glBindTexture(GL_TEXTURE_2D, texture_plane);

	// //Unuse Shader
	// glUseProgram(0);

	//Apply Camera Manipluator to Set Model View Matrix on GPU
   // ModelViewMatrix.toIdentity();
    
//changed cameraManip -> without
	Matrix4x4 m = ModelViewMatrix;
	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    m.getPtr());	        //Pointer to Matrix Values

	//Allows for camera to follow plane
		Matrix4x4 m4 = ModelViewMatrix;
		m4.translate(0.0,0.05,-1.0); //find camera again
		m4.rotate(180,0.0,0.5,0.0); //change vector and angle
	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    m4.getPtr());	        //Pointer to Matrix Values
    

	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_plane);
	glUniform1i(TextureMapUniformLocation, 0);


    //Call Draw Geometry Function
    mesh_plane.Draw(vertexPositionAttribute,vertexNormalAttribute,vertexTexcoordAttribute);
   
   	Matrix4x4 m1 = ModelViewMatrix;
	   m1.translate(0.0,0.1,0.0);
	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    m1.getPtr());	        //Pointer to Matrix Values
	
	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_ground);
	glUniform1i(TextureMapUniformLocation, 0);

	//vertexNormalAttribute -> -1
	mesh_ground.Draw(vertexPositionAttribute, -1 ,vertexTexcoordAttribute);
	

   	Matrix4x4 m2 = ModelViewMatrix;
	   m2.scale(10.0,10.0,10.0);
	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    m2.getPtr());	        //Pointer to Matrix Values
	
	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_sky);
	glUniform1i(TextureMapUniformLocation, 0);

   	mesh_sky.Draw(vertexPositionAttribute,-1,vertexTexcoordAttribute);
	
	glFlush();

     	//Unuse Shader                                                          
        glUseProgram(0);

    	//Swap Buffers and post redisplay
	glutSwapBuffers();
	glutPostRedisplay();
}



//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}
	else if(key == 'a')
        {
         	std::cout << "a key pressed" << std::endl;
        }
	else if(key == 'b')
	{
                glClearColor(1.0,1.0,1.0,1.0);
        }
        else if(key == 'B')
        {
                glClearColor(0.0,0.0,0.0,1.0);
	}
    	else if(key == 'w')
    	{
        	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    	}
    	else if(key == 'W')
    	{
     		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    	}

     	//Tell opengl to redraw frame                                           
	glutPostRedisplay();

    
    //Set key status
    keyStates[key] = true;
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{
    //keys should be handled here
	if(keyStates['a'])
    {
        
    }
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
   // cameraManip.handleMouse(button, state,x,y);
    //glutPostRedisplay();
}

//! Motion
void motion(int x, int y)
{
    cameraManip.handleMouseMotion(x,y);
    glutPostRedisplay();
}


//! Timer Function
void Timer(int value)
{
	// Do something
    
    //Call function again after 10 milli seconds
	glutTimerFunc(10,Timer, 0);
}



