
//Universaali aika, kaikille fragmenteille sama
uniform float time;

//location lasketaan automaattisesti interpoloimalla verteksien antamat arvot
varying vec2 location;

void main()
{
    float white;
    vec4 color;
    vec2 plasmaloc;

    plasmaloc.x = location.x + cos(time * 0.1) * 100;
    plasmaloc.y = location.y + sin(time * 0.1) * 70;

    plasmaloc /= 50.0;

    //Jonkinlainen plasmaefektilasku
    white = sin(plasmaloc.x) * sin(plasmaloc.y) * 2.5
          + sin(plasmaloc.x*(20 + 5*cos(plasmaloc.y*5))) * 1.5
          + sin(plasmaloc.y*(30 + 7*cos(plasmaloc.x*3))) * 1.5;

    white = abs(sin(white));

    //Määritellään fragmentin lopullinen väri
    color.x = (1 - white) * abs(sin(plasmaloc.y));
    color.y = white;
    color.z = 0.5 - 0.5 * white + 0.5;
    color.w = 1.0;

    //Kerrotaan vielä OpenGL:lle väri...
    gl_FragColor = color;
}
