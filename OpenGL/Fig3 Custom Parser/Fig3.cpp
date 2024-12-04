#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <fstream>
#include <sstream>
#include "getBMP.h"

using namespace std;

struct Material {
    string name;
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;
    string diffuseTextureFile;
};


struct Point {
    Point(glm::vec3 setOutV, glm::vec2 setOutUV, glm::vec3 setOutN , int matches) {
        outV = setOutV;
        outUV = setOutUV;
        outN = setOutN;
        f_sz = matches;
    }

    glm::vec3 outV;
    glm::vec2 outUV;
    glm::vec3 outN;
    int f_sz;
};

//Global Variables     //////////////////////////////////////////////////////////////
char tmpname[20];  // to know in each f group what is the name of group
map<string, vector<Point *> > PartsMap;
char mtlPath[128];
vector<glm::vec3> outVertices;
vector<glm::vec2> outUVs;
vector<glm::vec3> outNormals;
vector<Material> materials;
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate object.
static unsigned int texture[5]; // Array of texture indices.

vector<glm::vec3> GetOutVertices() {
    return outVertices;
}

vector<glm::vec2> GetOutUVs() {
    return outUVs;
}

vector<glm::vec3> GetOutNormals() {
    return outNormals;
}

vector<Material> GetMaterials() {
    return materials;
}

///////////////////////////////////////////////////////////////////////////////////////
static FILE *file(const char *path) {
    FILE *fp;
    if (fopen_s(&fp, path, "r") != 0) {
        printf("Impossible to open the file!\n");
        return NULL;
    }
    return fp;
}

bool ObjectLoader(const char *path) {
    //TEMPORARY DATA
    vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    vector<glm::vec3> temp_vertices;
    vector<glm::vec2> temp_uvs;
    vector<glm::vec3> temp_normals;

    //OPEN FILE
    FILE *f = file(path);
    if (f == NULL) {
        cout << "File not found" << '\n';
        return 0;
    }

    //READ FILE
    int cycle = 1;
    int matches;
    while (1) {
        char lineHeader[128];
        int res = fscanf(f, "%s", lineHeader, sizeof(lineHeader));
        if (res == -1)
            break;
        //cout<<"Cycle:"<<cycle<<" Res:" << res << '\n';
        cycle++;

        //READ MTL FILE
        if (strcmp(lineHeader, "rain_rig.mtl") == 0) {
            strcpy_s(mtlPath, lineHeader);
            cout << "mtlPath" << mtlPath << endl;
        }

        //READ VERTEX DATA
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(f, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            //cout<<"Lineheader: "<< lineHeader << ", Vertex:" << vertex.x << ' ' << vertex.y << ' ' << vertex.z << '\n';
            temp_vertices.push_back(vertex);
        }
            //READ UV DATA
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(f, "%f %f\n", &uv.x, &uv.y);
            //cout<<"vt:"<<uv.x<<' '<<uv.y<<'\n';
            temp_uvs.push_back(uv);
        }
            //READ NORMAL DATA
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(f, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            //cout<<"vn:"<<normal.x<<' '<<normal.y<<' '<<normal.z<<'\n';
            temp_normals.push_back(normal);
        }
            // Read Name of the mtl group
        else if (strcmp(lineHeader, "usemtl") == 0) {
            fscanf(f, "%s \n", &tmpname);
            // TODO Sheel el comments
            //    cout<<"A7a "<<tmpname<<endl;
        }
            //READ FACE DATA
        else if (strcmp(lineHeader, "f") == 0) {
            //    cout<<"A7a "<<tmpname<<endl;
            string vertex1, vertex2, vertex3, vertex4;
            unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
            matches = fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                             &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2],
                             &normalIndex[2] ,&vertexIndex[3], &uvIndex[3], &normalIndex[3] );
            //     cout<<"Matches : "<<matches<<" for "<<tmpname<<endl;
//            if (matches != 9) {
//                printf("File can't be read by our simple parser : ( Try exporting with other options \n");
//                return false;
//            }

            for (int i = 0; i < 4; i++) {
                //  cout<<temp_vertices[vertexIndex[i]-1].x<<" "<<temp_vertices[vertexIndex[i]-1].y<<" "<<temp_vertices[vertexIndex[i]-1].z<<endl;
                Point *tmpBuffer = new Point(temp_vertices[vertexIndex[i] - 1], temp_uvs[uvIndex[i] - 1],
                                             temp_normals[normalIndex[i] - 1],matches);
                //   cout<<tmpBuffer->outV.x<<" "<<tmpBuffer->outV.y<<" "<<tmpBuffer->outV.z<<endl;
                PartsMap[tmpname].push_back(tmpBuffer);
            }
        }
    }
    return 1;
}

bool MtlLoader(const char *path, vector<Material> &materials) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "File not found: " << path << endl;
        return false;
    }

    vector<Material> tempMaterials;
    Material currentMaterial;

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string token;
        iss >> token;

        if (token == "newmtl") {
            // Start of a new material definition
            if (currentMaterial.name != "") {
                tempMaterials.push_back(currentMaterial);
            }
            iss >> currentMaterial.name;
        } else if (token == "Ka") {
            // Ambient color
            iss >> currentMaterial.ambientColor.x >> currentMaterial.ambientColor.y >> currentMaterial.ambientColor.z;
        } else if (token == "Kd") {
            // Diffuse color
            iss >> currentMaterial.diffuseColor.x >> currentMaterial.diffuseColor.y >> currentMaterial.diffuseColor.z;
        } else if (token == "Ks") {
            // Specular color
            iss >> currentMaterial.specularColor.x >> currentMaterial.specularColor.y
                >> currentMaterial.specularColor.z;
        } else if (token == "Ns") {
            // Shininess
            iss >> currentMaterial.shininess;
        } else if (token == "map_Kd") {
            // Diffuse texture file
            iss >> currentMaterial.diffuseTextureFile;
        }
    }

    // Add the last material
    if (currentMaterial.name != "") {
        tempMaterials.push_back(currentMaterial);
    }

    file.close();

    // Copy the temporary materials to the output vector
    materials = tempMaterials;

    return true;
}


void loadTextures() {
    // Local storage for bmp image data.
    imageFile *image[5];

    // Load the images.
    image[0] = getBMP("GrimmchildTexture.bmp");
    // Bind grass image to texture object texture[0].
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}


// Drawing routine.
void drawScene() {
    vector<glm::vec3> outVertices = GetOutVertices();
    vector<glm::vec2> outUVs = GetOutUVs();
    vector<glm::vec3> outNormals = GetOutNormals();
    vector<Material> materials = GetMaterials();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    //   gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, -100.0, 0.0, 1.0, 0.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glRotatef(Zangle, 0.0, 0.0, 1.0);
    glRotatef(Yangle, 0.0, 1.0, 0.0);
    glRotatef(Xangle, 1.0, 0.0, 0.0);

    for (auto &it:PartsMap) {
        for (int i = 0; i < it.second.size(); i += 4) { // Looping on each part
            //     glEnable(GL_TEXTURE_2D);
            Material currentMaterial;
            for (int j = 0; j < materials.size(); j++) {
                if (it.first == materials[j].name) {
                    currentMaterial = materials[j];
                    break;
                }
            }
            //    cout<<"heya "<<currentMaterial.name<<" bas actually "<<it.first<<endl;
//            if (currentMaterial.diffuseTextureFile == ".\\GrimmchildTexture.png") {
//                //  cout<<"1"<<endl;
//                glBindTexture(GL_TEXTURE_2D, texture[0]);
//            }
            glBegin(GL_QUADS);
            glColor3f(currentMaterial.diffuseColor.x, currentMaterial.diffuseColor.y, currentMaterial.diffuseColor.z);
            // Vertex 1
            //  glTexCoord2f(it.second[i]->outUV.x, it.second[i]->outUV.y);
            glNormal3f(it.second[i]->outN.x, it.second[i]->outN.y, it.second[i]->outN.z);
            glVertex3f(it.second[i]->outV.x, it.second[i]->outV.y, it.second[i]->outV.z);

            // Vertex 2
            //    glTexCoord2f(it.second[i + 1]->outUV.x, it.second[i + 1]->outUV.y);
            glNormal3f(it.second[i + 1]->outN.x, it.second[i + 1]->outN.y, it.second[i + 1]->outN.z);
            glVertex3f(it.second[i + 1]->outV.x, it.second[i + 1]->outV.y, it.second[i + 1]->outV.z);

            // Vertex 3
            //     glTexCoord2f(it.second[i + 2]->outUV.x, it.second[i + 2]->outUV.y);
            glNormal3f(it.second[i + 2]->outN.x, it.second[i + 2]->outN.y, it.second[i + 2]->outN.z);
            glVertex3f(it.second[i + 2]->outV.x, it.second[i + 2]->outV.y, it.second[i + 2]->outV.z);
            // Vertex 4
            //  glTexCoord2f(it.second[i + 3]->outUV.x, it.second[i + 3]->outUV.y);
            glNormal3f(it.second[i + 3]->outN.x, it.second[i + 3]->outN.y, it.second[i + 3]->outN.z);
            glVertex3f(it.second[i + 3]->outV.x, it.second[i + 3]->outV.y, it.second[i + 3]->outV.z);

            glEnd();
            //   glDisable(GL_TEXTURE_2D);
            //    glMatrixMode(GL_MODELVIEW);
        }
    }


////////////////////


    glFlush();
}


// Initialization routine.
void setup(void) {

//    //////////////////
//    glGenTextures(1, texture);
//
//    // Load external textures.
//    loadTextures();
//
//    // Specify how texture values combine with current surface color values.
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//
//    // Turn on OpenGL texturing.
//    glEnable(GL_TEXTURE_2D);
//    /////////////////

    // Material property vectors.
    float matSpec[] = {0.0, 1.0, 1.0, 1.0};
    float matShine[] = {50.0};
    float matAmbAndDif[] = {0.0, 0.1, 1.0, 1.0};

    // Light property vectors.
    float lightAmb[] = {0.0, 0.1, 1.0, 1.0};
    float lightDifAndSpec[] = {0.0, 0.1, 1.0, 1.0};
    float lightPos[] = {0.0, 7.0, 3.0, 0.0};
    float globAmb[] = {0.2, 0.2, 0.2, 1.0};

    // Material properties of the objects.
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matAmbAndDif);

    // Light0 properties.
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Poperties of the ambient light.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.

    // Turn on OpenGL texturing.
    glEnable(GL_TEXTURE_2D);


    glEnable(GL_LIGHTING); // Enable lighting calculations.
    glEnable(GL_LIGHT0); // Enable particular light source.
    glEnable(GL_DEPTH_TEST); // Enable depth testing.

    glEnable(GL_NORMALIZE); // Enable automatic normalization of normals.

    glClearColor(1.0, 1.0, 1.0, 0.0);
}

// OpenGL window reshape routine.
void resize(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1.0, -1, 2.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Keyboard input processing routine.x
void keyInput(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
            break;
        case 'x':
            Xangle += 5.0;
            if (Xangle > 360.0) Xangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'X':
            Xangle -= 5.0;
            if (Xangle < 0.0) Xangle += 360.0;
            glutPostRedisplay();
            break;
        case 'y':
            Yangle += 5.0;
            if (Yangle > 360.0) Yangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'Y':
            Yangle -= 5.0;
            if (Yangle < 0.0) Yangle += 360.0;
            glutPostRedisplay();
            break;
        case 'z':
            Zangle += 5.0;
            if (Zangle > 360.0) Zangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'Z':
            Zangle -= 5.0;
            if (Zangle < 0.0) Zangle += 360.0;
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

void specialKeyInput(int key, int x, int y) {
    Xangle = Yangle = Zangle = 0.0;
    glutPostRedisplay();
}

void printInteraction(void) {
    std::cout << "Interaction:" << std::endl;
    std::cout << "Press the arrow keys to cycle through the objects." << std::endl
              << "Press x, X, y, Y, z, Z to turn the objects." << std::endl;
}


int main(int argc, char **argv) {
    bool res = ObjectLoader("./rain_rig.obj");
    bool res2 = MtlLoader(mtlPath, materials);
    if (res == 0) {
        cout << "Error 1" << '\n';
        return 0;
    } else {
        cout << "Success 1" << '\n';
//        cout << "Vertices:" << outVertices.size() << '\n';
//        cout << "UVs:" << outUVs.size() << '\n';
//        cout << "Normals:" << outNormals.size() << '\n';
        cout<<" GrimmChild : "<<PartsMap["GrimmChild"].size()<<endl;
    }
    if (res2 == 0) {
        cout << "Error 2" << '\n';
        return 0;
    } else {
        cout << "Success 2" << '\n';
        cout << "Materials:" << materials.size() << '\n';
    }

    // Print the data in the Material vector
    for (unsigned int i = 0; i < materials.size(); i++) {
        cout << "Material " << i << ":" << endl;
        cout << "Name: " << materials[i].name << endl;
        cout << "Ambient color: " << materials[i].ambientColor.x << " " << materials[i].ambientColor.y << " "
             << materials[i].ambientColor.z << endl;
        cout << "Diffuse color: " << materials[i].diffuseColor.x << " " << materials[i].diffuseColor.y << " "
             << materials[i].diffuseColor.z << endl;
        cout << "Specular color: " << materials[i].specularColor.x << " " << materials[i].specularColor.y << " "
             << materials[i].specularColor.z << endl;
        cout << "Shininess: " << materials[i].shininess << endl;
        cout << "Diffuse texture file: " << materials[i].diffuseTextureFile << endl;
        cout << endl;
    }

    // Use the data in opengl to draw the object
    printInteraction();

    glutInit(&argc, argv);

    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Parser.cpp");

    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);

    glewExperimental = GL_TRUE;
    glewInit();

    setup();

    glutMainLoop();

    return 0;
}