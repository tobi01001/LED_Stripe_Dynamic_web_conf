#ifndef FAVICON_H
#define FAVICON_H

#define FAVICON_SVG_PART_1 (F("<svg width='400px' height='400px' xmlns='http://www.w3.org/2000/svg'> <radialGradient id='red'> <stop offset='30%' stop-color='rgb(255, 0, 0)'/> <stop offset='70%' stop-color='rgba(128, 0, 0, 0.5)'/> <stop offset='100%' stop-color='#3700b300'/> </radialGradient> <radialGradient id='yellow'> <stop offset='30%' stop-color='rgb(255, 255, 0)'/> <stop offset='70%' stop-color='rgba(128, 128, 0, 0.5)'/> <stop offset='100%' stop-color='#3700b300'/> </radialGradient> <radialGradient id='green'> <stop offset='30%' stop-color='rgb(0, 255, 0)'/> <stop offset='70%' stop-color='rgba(0, 128, 0, 0.5)'/> <stop offset='100%' stop-color='#3700b300'/> </radialGradient> <radialGradient id='blue'> <stop offset='30%' stop-color='rgb(0, 0, 255)'/> <stop offset='70%' stop-color='rgba(0, 0, 128, 0.5)'/> <stop offset='100%' stop-color='#3700b300'/> </radialGradient> <rect rx='10' x='0' y='0' width='400' height='400' fill='#0000007f'/> <ellipse cx='20' cy='20' rx='8' ry='8' fill='url(#red)'/> <ellipse cx='40' cy='20' rx='8' ry='8' fill='url(#red)'/> <ellipse cx='60' cy='20' rx='8' ry='8' fill='url(#red)'/> <ellipse cx='340' cy='380' rx='8' ry='8' fill='url(#yellow)'/> <ellipse cx='360' cy='380' rx='8' ry='8' fill='url(#yellow)'/> <ellipse cx='380' cy='380' rx='8' ry='8' fill='url(#yellow)'/> <ellipse cx='20' cy='340' rx='8' ry='8' fill='url(#green)'/> <ellipse cx='20' cy='360' rx='8' ry='8' fill='url(#green)'/> <ellipse cx='20' cy='380' rx='8' ry='8' fill='url(#green)'/> <ellipse cx='380' cy='20' rx='8' ry='8' fill='url(#blue)'/> <ellipse cx='380' cy='40' rx='8' ry='8' fill='url(#blue)'/> <ellipse cx='380' cy='60' rx='8' ry='8' fill='url(#blue)'/> <text class='strong' x='58' y='138' font-size='130' id='svg_1' fill='#000000'> LED </text> <text class='strong' x='50' y='130' font-size='130' id='svg_1' fill='#00ff00'> LED </text> <text class='strong' font-size='240' y='350' x='30' fill='#000000'>"))

#define FAVICON_SVG_PART_2 (F("</text> <text class='strong' font-size='240' y='340' x='20' fill='#f0f0f0'>"))

#define FAVICON_SVG_PART_3 (F("</text></svg>"))

#ifndef ICON_LETTERS
    #define ICON_LETTERS (F("CT"))
#endif

#endif // FAVICON_H