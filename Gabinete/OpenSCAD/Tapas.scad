// ===========================================================================
//  Copyright (C) 2021   Walter Pirri
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ===========================================================================

// ===========================================================================
// Tapa superior de la caja para central de domotica
// HELP: https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Primitive_Solids
// ===========================================================================

inicial=[0,0,0];

// Medidas de la tapa
ancho=180;
alto_tapa_superior=100;
alto_tapa_inferior=50;
espesor_tapa=2;
pared_caja=2;

// Detalles de la tapa
ancho_borde=1;          // (pared/2) de caja
altura_borde=1.5;       // 1/2 mm menos que profundidad_calado en la caja
altura_protector=25;    // Pared de separacion que queda detrás de las borneras

// Altura sobre la pestaña donde va el orificio del tornillo de sujeccion
// Relacionado con pos_tornillo_h de la caja
pos_tornillo_h=10;

// Pestañas de sujeccion
espesor_guias=2;
largo_guias=15;
ancho_guias=10;

// Tapa lateral que oculta parcialmente la ventana de acceso a los puertos
// de la Raspberry Pi. Relacionados con ancho_ventana y 
// posicion_ventana de la caja
posicion_tapa_lateral=29;
ancho_tapa_lateral=40;
altura_tapa_lateral=18;

// Pocoion del angulo superios derecho del display
ancho_display=113.11;
alto_display=70;
// En la caja: x_display=(ancho/2) - (ancho_display/2);
//             y_display=pared+5;
x_display=0 + (ancho/2) - (ancho_display/2);
y_display=5 + pared_caja + 5;

// trabas entre las tapas
posicion_trabas=20;
ancho_trabas_tapa_superior=3;
ancho_trabas_tapa_inferior=1;
largo_trabas_tapa_inferior=2;


module bordes(xyz, b, h, hb1, hb2, hb3, hb4)
{
    translate(xyz) cube([b, ancho_borde, hb1], false);
    translate(xyz) cube([ancho_borde, h, hb2], false);
    translate(xyz + [b-ancho_borde,0,0]) cube([ancho_borde, h, hb3], false);
    
    translate(xyz + [espesor_tapa,h-espesor_tapa,0]) cube([b - (2*espesor_tapa), espesor_tapa, hb4], false);
}

module oreja(xyz)
{
    difference()
    {
        union()
        {
            // Pestañas
            translate(xyz) cube([espesor_guias, ancho_guias, largo_guias - (ancho_guias/2)], false);
            translate(xyz + [0,(ancho_guias/2),largo_guias - (ancho_guias/2)])
                rotate(a=[0,90,0])
                cylinder(espesor_guias, r = (ancho_guias/2), $fn = 100);
        }
        // Agujeros
        translate(xyz + [0,(ancho_guias/2),pos_tornillo_h])
            rotate(a=[0,90,0])
            cylinder(espesor_guias, r = 1, $fn = 100);
    }

}


module frente_tapa_superior(xyz)
{
    translate(xyz) cube([ancho, alto_tapa_superior, espesor_tapa], false);

    bordes(xyz + [0,0,espesor_tapa], ancho, alto_tapa_superior, ancho_borde, ancho_borde, ancho_borde, altura_protector);

    oreja(xyz + [espesor_tapa,12,espesor_tapa]);
    oreja(xyz + [espesor_tapa,72,espesor_tapa]);
    oreja(xyz + [ancho - (espesor_tapa + espesor_guias),12,espesor_tapa]);
    oreja(xyz + [ancho - (espesor_tapa + espesor_guias),72,espesor_tapa]);

    translate(xyz + [0,posicion_tapa_lateral,espesor_tapa])
		cube([espesor_tapa, ancho_tapa_lateral, altura_tapa_lateral], false);
}

module ventana_display(xyz)
{
    translate(xyz + [x_display, y_display, 0.5]) cube([ancho_display, alto_display, espesor_tapa], false);
}

module trabas_tapa_superior(xyz)
{
	translate(xyz + [posicion_trabas-(ancho_trabas_tapa_superior/2),(alto_tapa_superior-espesor_guias),espesor_tapa])
		cube([ancho_trabas_tapa_superior, espesor_guias, 5]); 
	translate(xyz + [ancho-posicion_trabas-(ancho_trabas_tapa_superior/2),(alto_tapa_superior-espesor_guias),espesor_tapa])
		cube([ancho_trabas_tapa_superior, espesor_guias, 5]); 
}


module frente_tapa_inferior(xyz)
{
    translate(xyz) cube([ancho, alto_tapa_inferior, espesor_tapa], false);

    bordes(xyz + [0,0,espesor_tapa], ancho, alto_tapa_inferior, ancho_borde, ancho_borde, ancho_borde, ancho_borde);

    oreja(xyz + [espesor_tapa,5,espesor_tapa]);
    oreja(xyz + [ancho - (espesor_tapa + espesor_guias),5,espesor_tapa]);
}

module trabas_tapa_inferior(xyz)
{
	translate(xyz + [posicion_trabas-(ancho_trabas_tapa_inferior/2),alto_tapa_inferior,espesor_tapa])
		cube([ancho_trabas_tapa_inferior, largo_trabas_tapa_inferior, altura_borde]); 
	translate(xyz + [ancho-posicion_trabas-(ancho_trabas_tapa_inferior/2),alto_tapa_inferior,espesor_tapa])
		cube([ancho_trabas_tapa_inferior, largo_trabas_tapa_inferior, altura_borde]); 
}



module TapaSuperior()
{
	difference()
	{
		frente_tapa_superior(inicial);
		ventana_display(inicial);
		trabas_tapa_superior(inicial);
	}
}

module TapaInferior()
{
	frente_tapa_inferior(inicial);
	trabas_tapa_inferior(inicial);

}

//
// Construcción
//

//TapaSuperior();

TapaInferior();