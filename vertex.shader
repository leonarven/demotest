//time on määriteltu main.cpp tiedostossa
//uniform tyyppi tarkoittaa, että se näkyy kaikille shadereille samana
uniform float time;

//varying muuttuja location välitetään verteksiltä fragmenteille
//fragmentin saama arvo on interpoloitu sen muodostaneiden verteksien
//arvoista
varying vec2 location;

void main()
{
    //Talletetaan vektoriin v verteksin sijainti
    vec4 v = vec4(gl_Vertex);

    //Muokataan verteksin z-koordinaattia
    v.z = v.z + sin(0.3*v.x + time) * 5;

    //Asetetaan verteksin x ja y -koordinaatit location muuttujaan
    //location vektori välittyy interpoloituna fragmenteille
    location.x = v.x;
    location.y = v.y;

    //Verteksin lopullinen sijainti saadaan kertomalla se
    //projektiomatriisin kanssa
    gl_Position = gl_ModelViewProjectionMatrix * v;
}
