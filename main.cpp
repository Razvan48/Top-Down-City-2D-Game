#include <iostream>
#include <glew.h>
#include <glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <fstream>

#define f_pi atan(1) * 4.0f

using namespace std;

const char* c_sursa_shader_vertex =
"#version 330 core \n"
"\n"
"layout (location = 0) in vec3 coordonate_varf; \n"
"layout (location = 1) in vec2 coordonate_initiale_textura; \n"
"\n"
"out vec2 coordonate_textura; \n"
"\n"
"uniform mat4 matrice_model; \n"
"uniform mat4 matrice_vedere; \n"
"uniform mat4 matrice_proiectie; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"gl_Position = matrice_proiectie * matrice_vedere * matrice_model * vec4 (coordonate_varf, 1.0f); \n"
"coordonate_textura = coordonate_initiale_textura; \n"
"\n"
"} \n";

const char* c_sursa_shader_fragment =
"#version 330 core \n"
"\n"
"in vec2 coordonate_textura; \n"
"out vec4 coordonate_fragment; \n"
"\n"
"uniform sampler2D textura; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"coordonate_fragment = texture(textura, coordonate_textura); \n"
"\n"
"} \n";

int i_latime_fereastra_joc = 1024;
int i_inaltime_fereastra_joc = 768;

int program_shader;
int shader_vertex;
int shader_fragment;

int loc_matrice_model;
int loc_matrice_vedere;
int loc_matrice_proiectie;
int loc_textura;

float f_timp_cadru_anterior = 0.0f;
float f_durata_timp_cadru;

float f_distanta_minima_vedere = 1.0f;
float f_distanta_maxima_vedere = 128.0f;
float f_dimensiune_minima_camp_vizual = 1.0f;
float f_dimensiune_maxima_camp_vizual = 45.0f;

int i_grosime_contur_obiecte = 3;

float f_dimensiune_obiect = 1.0f;

float f_dimensiune_camp_vizual = 45.0f;

float f_x_jucator = 0.5f;
float f_y_jucator = 0.5f;
float f_unghi_jucator = 0.0f;
float f_viteza_jucator = 2.5f;
float f_viteza_unghiulara_jucator = f_pi;

glm::vec3 coordonate_camera = glm::vec3(0.5f, 0.5f, 3.0f);
glm::vec3 directie_privire_camera = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 normala_camera = glm::vec3(-1.0f, 0.0f, 0.0f);

int i_x_harta = 128;
int i_y_harta = 128;
int i_harta_joc[128][128];

/*
    Conventie:
    void = -3;
    apa = -2;
    iarba = -1;
    strada = 0;
    1 <= cladire_1 <= 100;
    101 <= cladire_2 <= 200;
    201 <= cladire_3 <= 300;
    s.a.m.d.;
*/

struct textura
{
    unsigned int ui_id_textura;
    int i_latime, i_inaltime, i_nr_canale;
};

void v_modificare_dimensiuni_fereastra_joc(GLFWwindow* fereastra_joc, int i_latime_fereastra_noua_joc, int i_inaltime_fereastra_noua_joc)
{
    glViewport(0, 0, i_latime_fereastra_noua_joc, i_inaltime_fereastra_noua_joc);
}

bool b_exista_spatiu(float f_x_test, float f_y_test)
{
    int i_x_test = (int)f_x_test;
    int i_y_test = (int)f_y_test;
    if (i_harta_joc[i_x_test][i_y_test] >= 1)
    {
        return false;
    }
    return true;
}

textura apa, cladire_1_etaj, cladire_1_parter, cul_de_sac_iesire_est, cul_de_sac_iesire_nord, cul_de_sac_iesire_sud, cul_de_sac_iesire_vest, curba_nord_est, curba_nord_vest, curba_sud_est, curba_sud_vest, iarba, intersectie_3_nord_est_vest, intersectie_3_nord_sud_est, intersectie_3_nord_sud_vest, intersectie_3_sud_est_vest, intersectie_4, nimic, strada_0, strada_orizontala, strada_verticala;
textura vehicul_jucator;

bool b_setare_textura_strada(int i, int j)
{
    //glBindTexture(GL_TEXTURE_2D, textura_pe_care_o_vrem);
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, cul_de_sac_iesire_est.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] != 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i - 1][j] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, cul_de_sac_iesire_nord.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] != 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i - 1][j] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, cul_de_sac_iesire_sud.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, cul_de_sac_iesire_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, curba_nord_est.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, curba_nord_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, curba_sud_est.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, curba_sud_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, intersectie_3_nord_est_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, intersectie_3_nord_sud_est.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, intersectie_3_nord_sud_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, intersectie_3_sud_est_vest.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, intersectie_4.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, strada_0.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] != 0 && i_harta_joc[i][j - 1] == 0 && i_harta_joc[i + 1][j] != 0 && i_harta_joc[i][j + 1] == 0)
    {
        glBindTexture(GL_TEXTURE_2D, strada_orizontala.ui_id_textura);
        return true;
    }
    if (i_harta_joc[i - 1][j] == 0 && i_harta_joc[i][j - 1] != 0 && i_harta_joc[i + 1][j] == 0 && i_harta_joc[i][j + 1] != 0)
    {
        glBindTexture(GL_TEXTURE_2D, strada_verticala.ui_id_textura);
        return true;
    }
    return false;
}

void v_date_intrare_tastatura(GLFWwindow* fereastra_joc)
{
    if (glfwGetKey(fereastra_joc, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(fereastra_joc, 1);
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_W) == GLFW_PRESS)
    {
        float f_x_urmator = f_x_jucator - f_viteza_jucator * f_durata_timp_cadru * sin(f_unghi_jucator);
        float f_y_urmator = f_y_jucator + f_viteza_jucator * f_durata_timp_cadru * cos(f_unghi_jucator);
        //if (b_exista_spatiu(f_x_urmator, f_y_urmator))
        //{
        f_x_jucator = f_x_urmator;
        f_y_jucator = f_y_urmator;
        coordonate_camera.x = f_x_jucator;
        coordonate_camera.y = f_y_jucator;
        //}
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_S) == GLFW_PRESS)
    {
        float f_x_urmator = f_x_jucator + f_viteza_jucator * f_durata_timp_cadru * sin(f_unghi_jucator);
        float f_y_urmator = f_y_jucator - f_viteza_jucator * f_durata_timp_cadru * cos(f_unghi_jucator);
        //if (b_exista_spatiu(f_x_urmator, f_y_urmator))
        //{
        f_x_jucator = f_x_urmator;
        f_y_jucator = f_y_urmator;
        coordonate_camera.x = f_x_jucator;
        coordonate_camera.y = f_y_jucator;
        //}
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_A) == GLFW_PRESS)
    {
        f_unghi_jucator += f_viteza_unghiulara_jucator * f_durata_timp_cadru;
        while (f_unghi_jucator >= 2 * f_pi) //folosim while in cazul in care lucram cu viteze de rotatie extrem de mari;
        {
            f_unghi_jucator -= 2.0f * f_pi;
        }
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_D) == GLFW_PRESS)
    {
        f_unghi_jucator -= f_viteza_unghiulara_jucator * f_durata_timp_cadru;
        while (f_unghi_jucator < 0.0f) //folosim while in cazul in care lucram cu viteze de rotatie extrem de mari;
        {
            f_unghi_jucator += 2.0f * f_pi;
        }
    }
    /*
    if (glfwGetKey(fereastra_joc, GLFW_KEY_H) == GLFW_PRESS)
    {
        for (int i = 0; i < i_x_harta; i++)
        {
            for (int j = 0; j < i_y_harta; j++)
            {
                cout << i_harta_joc[i][j] << ' ';
            }
            cout << endl;
        }
    }
    */
    int i_x_jucator = (int)f_x_jucator;
    int i_y_jucator = (int)f_y_jucator;
    if (i_x_jucator >= 1 && i_x_jucator < i_x_harta - 1 && i_y_jucator >= 1 && i_y_jucator < i_y_harta - 1)
    {
        if (glfwGetKey(fereastra_joc, GLFW_KEY_X) == GLFW_PRESS)
        {
            i_harta_joc[i_x_jucator][i_y_jucator] = -3;
        }
        if (glfwGetKey(fereastra_joc, GLFW_KEY_B) == GLFW_PRESS)
        {
            i_harta_joc[i_x_jucator][i_y_jucator] = 0;
        }
        if (glfwGetKey(fereastra_joc, GLFW_KEY_I) == GLFW_PRESS)
        {
            i_harta_joc[i_x_jucator][i_y_jucator] = -1;
        }
        if (glfwGetKey(fereastra_joc, GLFW_KEY_O) == GLFW_PRESS)
        {
            i_harta_joc[i_x_jucator][i_y_jucator] = -2;
        }
        if (glfwGetKey(fereastra_joc, GLFW_KEY_N) == GLFW_PRESS)
        {
            if (i_harta_joc[i_x_jucator][i_y_jucator] % 100 >= 2 && i_harta_joc[i_x_jucator][i_y_jucator] >= 1)
            {
                i_harta_joc[i_x_jucator][i_y_jucator] --;
            }
            else
            {
                i_harta_joc[i_x_jucator][i_y_jucator] = -3;
            }
        }
        if (glfwGetKey(fereastra_joc, GLFW_KEY_M) == GLFW_PRESS)
        {
            if (i_harta_joc[i_x_jucator][i_y_jucator] % 100 <= 98 && i_harta_joc[i_x_jucator][i_y_jucator] >= 1)
            {
                i_harta_joc[i_x_jucator][i_y_jucator] ++;
            }
            else
            {
                i_harta_joc[i_x_jucator][i_y_jucator] = 1;
            }
        }
    }
}

void v_date_intrare_scroll_mouse(GLFWwindow* fereastra_joc, double d_x_offset, double d_variatie_coordonata_scroll_mouse)
{
    coordonate_camera.z -= 100.0f * (float)d_variatie_coordonata_scroll_mouse * f_durata_timp_cadru;
    if (coordonate_camera.z <= f_distanta_minima_vedere)
    {
        coordonate_camera.z = f_distanta_minima_vedere + 0.0001f;
    }
    if (coordonate_camera.z >= f_distanta_maxima_vedere)
    {
        coordonate_camera.z = f_distanta_maxima_vedere - 0.0001f;
    }
}

void v_incarcare_textura(textura* textura, const char* adresa_textura)
{
    glGenTextures(1, &(textura->ui_id_textura));
    glBindTexture(GL_TEXTURE_2D, textura->ui_id_textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char* data = stbi_load(adresa_textura, &(textura->i_latime), &(textura->i_inaltime), &(textura->i_nr_canale), 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textura->i_latime, textura->i_inaltime, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_set_flip_vertically_on_load(true);
    stbi_image_free(data);
}

void v_initializare_harta(const char* adresa_harta)
{
    ifstream citire(adresa_harta);
    for (int i = 0; i < i_x_harta; i++)
    {
        for (int j = 0; j < i_y_harta; j++)
        {
            citire >> i_harta_joc[i][j];
        }
    }
}

void v_salvare_harta(const char* adresa_harta)
{
    ofstream scriere(adresa_harta);
    for (int i = 0; i < i_x_harta; i++)
    {
        for (int j = 0; j < i_y_harta; j++)
        {
            scriere << i_harta_joc[i][j] << ' ';
        }
        scriere << endl;
    }
}

int main()
{
    v_initializare_harta("resurse/harti/harta_1.txt");
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* fereastra_joc = glfwCreateWindow(i_latime_fereastra_joc, i_inaltime_fereastra_joc, "fereastra_joc", 0, 0);
    //glfwGetPrimaryMonitor();
    glfwMakeContextCurrent(fereastra_joc);
    glewInit();
    glfwSetFramebufferSizeCallback(fereastra_joc, v_modificare_dimensiuni_fereastra_joc);
    glfwSetScrollCallback(fereastra_joc, v_date_intrare_scroll_mouse);
    glEnable(GL_DEPTH_TEST);
    shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex, 1, &c_sursa_shader_vertex, 0);
    glCompileShader(shader_vertex);
    shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment, 1, &c_sursa_shader_fragment, 0);
    glCompileShader(shader_fragment);
    program_shader = glCreateProgram();
    glAttachShader(program_shader, shader_vertex);
    glAttachShader(program_shader, shader_fragment);
    glLinkProgram(program_shader);
    glDeleteShader(shader_vertex);
    glDeleteShader(shader_fragment);
    glUseProgram(program_shader);
    v_incarcare_textura(&apa, "resurse/texturi/apa.png");
    v_incarcare_textura(&cladire_1_etaj, "resurse/texturi/cladire_1_etaj.png");
    v_incarcare_textura(&cladire_1_parter, "resurse/texturi/cladire_1_parter.png");
    v_incarcare_textura(&cul_de_sac_iesire_est, "resurse/texturi/cul_de_sac_iesire_est.png");
    v_incarcare_textura(&cul_de_sac_iesire_nord, "resurse/texturi/cul_de_sac_iesire_nord.png");
    v_incarcare_textura(&cul_de_sac_iesire_sud, "resurse/texturi/cul_de_sac_iesire_sud.png");
    v_incarcare_textura(&cul_de_sac_iesire_vest, "resurse/texturi/cul_de_sac_iesire_vest.png");
    v_incarcare_textura(&curba_nord_est, "resurse/texturi/curba_nord_est.png");
    v_incarcare_textura(&curba_nord_vest, "resurse/texturi/curba_nord_vest.png");
    v_incarcare_textura(&curba_sud_est, "resurse/texturi/curba_sud_est.png");
    v_incarcare_textura(&curba_sud_vest, "resurse/texturi/curba_sud_vest.png");
    v_incarcare_textura(&iarba, "resurse/texturi/iarba.png");
    v_incarcare_textura(&intersectie_3_nord_est_vest, "resurse/texturi/intersectie_3_nord_est_vest.png");
    v_incarcare_textura(&intersectie_3_nord_sud_est, "resurse/texturi/intersectie_3_nord_sud_est.png");
    v_incarcare_textura(&intersectie_3_nord_sud_vest, "resurse/texturi/intersectie_3_nord_sud_vest.png");
    v_incarcare_textura(&intersectie_3_sud_est_vest, "resurse/texturi/intersectie_3_sud_est_vest.png");
    v_incarcare_textura(&intersectie_4, "resurse/texturi/intersectie_4.png");
    v_incarcare_textura(&nimic, "resurse/texturi/nimic.png");
    v_incarcare_textura(&strada_0, "resurse/texturi/strada_0.png");
    v_incarcare_textura(&strada_orizontala, "resurse/texturi/strada_orizontala.png");
    v_incarcare_textura(&strada_verticala, "resurse/texturi/strada_verticala.png");

    v_incarcare_textura(&vehicul_jucator, "resurse/texturi/vehicul_jucator.png");

    unsigned int vao_global;
    unsigned int vbo_global;
    glGenVertexArrays(1, &vao_global);
    glGenBuffers(1, &vbo_global);
    glBindVertexArray(vao_global);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_global);
    float f_coordonate_varfuri[] =
    {
        //FATA DE JOS, EXTREM DE IMPORTANTA
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 0.0f,

        -0.5f, 0.5f, -0.5f,   1.0f, 1.0f,
        0.5f, 0.5f, -0.5f,   1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 0.0f,

        //ALTE FETE:
        0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 1.0f / 3.0f,
        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        0.5f, 0.5f, -0.5f,   1.0f, 1.0f / 3.0f,

        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, 0.5f, -0.5f,   1.0f, 2.0f / 3.0f,
        0.5f, 0.5f, -0.5f,   1.0f, 1.0f / 3.0f,

        //

        0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 1.0f / 3.0f,
        0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 1.0f / 3.0f,
        0.5f, -0.5f, -0.5f,   1.0f / 3.0f, 0.0f,

        0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 1.0f / 3.0f,
        0.5f, 0.5f, -0.5f,   2.0f / 3.0f, 0.0f,
        0.5f, -0.5f, -0.5f,   1.0f / 3.0f, 0.0f,

        //

        0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 1.0f / 3.0f,
        -0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 2.0f / 3.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 1.0f / 3.0f,

        -0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 2.0f / 3.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 1.0f / 3.0f,

        //

        -0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, -0.5f, -0.5f,   1.0f / 3.0f, 1.0f,

        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, 0.5f, -0.5f,   2.0f / 3.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   1.0f / 3.0f, 1.0f,

        //

        -0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 2.0f / 3.0f,
        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 1.0f / 3.0f,

        -0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 2.0f / 3.0f,
        0.5f, 0.5f, 0.5f,   2.0f / 3.0f, 1.0f / 3.0f,
        0.5f, -0.5f, 0.5f,   1.0f / 3.0f, 1.0f / 3.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(f_coordonate_varfuri), f_coordonate_varfuri, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);
    loc_matrice_model = glGetUniformLocation(program_shader, "matrice_model");
    loc_matrice_vedere = glGetUniformLocation(program_shader, "matrice_vedere");
    loc_matrice_proiectie = glGetUniformLocation(program_shader, "matrice_proiectie");
    loc_textura = glGetUniformLocation(program_shader, "textura");
    glm::mat4 matrice_proiectie = glm::perspective(glm::radians(f_dimensiune_camp_vizual), (float)i_latime_fereastra_joc / (float)i_inaltime_fereastra_joc, 0.1f, f_distanta_maxima_vedere);
    glUniformMatrix4fv(loc_matrice_proiectie, 1, GL_FALSE, glm::value_ptr(matrice_proiectie));
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(loc_textura, 0);
    while (!glfwWindowShouldClose(fereastra_joc))
    {
        float f_timp_curent = (float)glfwGetTime();
        f_durata_timp_cadru = f_timp_curent - f_timp_cadru_anterior;
        f_timp_cadru_anterior = f_timp_curent;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 matrice_vedere = glm::lookAt(coordonate_camera, coordonate_camera + directie_privire_camera, normala_camera);
        glUniformMatrix4fv(loc_matrice_vedere, 1, GL_FALSE, glm::value_ptr(matrice_vedere));
        for (int i = 0; i < i_x_harta; i++)
        {
            for (int j = 0; j < i_y_harta; j++)
            {
                //glBindTexture(GL_TEXTURE_2D, textura_pe_care_o_vrem);
                float k = float(i);
                float l = float(j);
                if (i_harta_joc[i][j] >= 1)
                {
                    if (i_harta_joc[i][j] <= 99)
                    {
                        for (int m = 1; m <= i_harta_joc[i][j] % 100; m++)
                        {
                            float n = float(m);
                            glm::mat4 matrice_model = glm::mat4(1.0f);
                            matrice_model = glm::translate(matrice_model, glm::vec3((k + 0.5f) * f_dimensiune_obiect, (l + 0.5f) * f_dimensiune_obiect, (n - 0.5f) * f_dimensiune_obiect));
                            matrice_model = glm::scale(matrice_model, glm::vec3(f_dimensiune_obiect));
                            glUniformMatrix4fv(loc_matrice_model, 1, GL_FALSE, glm::value_ptr(matrice_model));
                            if (m == 1)
                            {
                                glBindTexture(GL_TEXTURE_2D, cladire_1_parter.ui_id_textura);
                            }
                            else
                            {
                                glBindTexture(GL_TEXTURE_2D, cladire_1_etaj.ui_id_textura);
                            }
                            glDrawArrays(GL_TRIANGLES, 0, 36);
                        }
                    }
                }
                else
                {
                    glm::mat4 matrice_model = glm::mat4(1.0f);
                    matrice_model = glm::translate(matrice_model, glm::vec3((k + 0.5f) * f_dimensiune_obiect, (l + 0.5f) * f_dimensiune_obiect, 0.5f * f_dimensiune_obiect));
                    matrice_model = glm::scale(matrice_model, glm::vec3(f_dimensiune_obiect));
                    glUniformMatrix4fv(loc_matrice_model, 1, GL_FALSE, glm::value_ptr(matrice_model));
                    if (i_harta_joc[i][j] == -3)
                    {
                        glBindTexture(GL_TEXTURE_2D, nimic.ui_id_textura);
                    }
                    if (i_harta_joc[i][j] == -2)
                    {
                        glBindTexture(GL_TEXTURE_2D, apa.ui_id_textura);
                    }
                    if (i_harta_joc[i][j] == -1)
                    {
                        glBindTexture(GL_TEXTURE_2D, iarba.ui_id_textura);
                    }
                    if (i_harta_joc[i][j] == 0)
                    {
                        b_setare_textura_strada(i, j);
                    }
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }
        }
        glBindTexture(GL_TEXTURE_2D, vehicul_jucator.ui_id_textura);
        glm::mat4 matrice_model = glm::mat4(1.0f);
        matrice_model = glm::translate(matrice_model, glm::vec3(f_x_jucator, f_y_jucator, 0.6f * f_dimensiune_obiect));
        matrice_model = glm::rotate(matrice_model, f_unghi_jucator, glm::vec3(0.0f, 0.0f, 1.0f));
        matrice_model = glm::scale(matrice_model, glm::vec3(f_dimensiune_obiect / 7.0f, f_dimensiune_obiect / 3.5f, 1.0f));
        glUniformMatrix4fv(loc_matrice_model, 1, GL_FALSE, glm::value_ptr(matrice_model));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        v_date_intrare_tastatura(fereastra_joc);
        glfwSwapBuffers(fereastra_joc);
        glfwPollEvents();
    }
    v_salvare_harta("resurse/harti/harta_1.txt");
    glDeleteBuffers(1, &vbo_global);
    glDeleteVertexArrays(1, &vao_global);
    glfwDestroyWindow(fereastra_joc);
    glfwTerminate();
    return 0;
}
