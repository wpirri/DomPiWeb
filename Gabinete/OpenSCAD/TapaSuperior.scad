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
ancho=160;
alto=100;
espesor=2;

// Detalles de la tapa
ancho_borde=1;          // (pared/2) de caja
altura_borde=1.5;       // 1/2 mm menos que profundidad_calado en la caja
altura_protector=20;    // Pared de separacion que queda detrás de las borneras

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
x_display=25;   // x_display de Caja.scad + 1.56
y_display=15;   // y_display de Caja.scad + 10
ancho_display=110;
alto_display=70;

module bordes(xyz, b, h)
{
    translate(xyz) cube([b, ancho_borde, altura_borde], false);
    translate(xyz) cube([ancho_borde, h, altura_borde], false);
    translate(xyz + [b-ancho_borde,0,0]) cube([ancho_borde, h, altura_borde], false);
    
    translate(xyz + [espesor,h-espesor,0]) cube([b - (2*espesor), espesor, altura_protector], false);
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


module frente(xyz)
{
    translate(xyz) cube([ancho, alto, espesor], false);

    bordes(xyz + [0,0,espesor], ancho, alto);

    oreja(xyz + [espesor,12,espesor]);
    oreja(xyz + [espesor,72,espesor]);
    oreja(xyz + [ancho - (espesor + espesor_guias),12,espesor]);
    oreja(xyz + [ancho - (espesor + espesor_guias),72,espesor]);

    translate(xyz + [0,posicion_tapa_lateral,espesor]) cube([espesor, ancho_tapa_lateral, altura_tapa_lateral], false);
}


//
// Construcción
//

difference()
{
    frente(inicial);
    translate(inicial + [x_display, y_display, 0]) cube([ancho_display, alto_display, espesor], false);
}