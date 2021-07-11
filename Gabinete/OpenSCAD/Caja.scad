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
// Cuerpo de la caja para central de domotica
// HELP: https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Primitive_Solids
// ===========================================================================

inicial=[0,0,0];

// Medidas de la caja
ancho=160;
alto=140;
profundidad=45;
pared=2;

// Detalles de la caja
profundidad_calado=2;   // Calado superior en los borede
ancho_ventana=40;       // ventana lateral para acceso a puertos de Raspberry Pi
posicion_ventana=29;

pos_tornillo_y=17;      // Tornillos de sujeccion para las pestañas de la tapa
pos_tornillo_h=36;
separacion_tornillos=60;
espesor_guias=1;        // Guias para las pestañas de la tapa
separacion_guias=11;

h_rbpi=3;               // Altura del soporte de Raspberry Pi
x_rbpi=58;              // Posicion del soporte superios derecho de Raspberry Pi
y_rbpi=30;

h_display=35;           // Altura del soporte del display
x_display=23.45;        // Posicion del soporte superios derecho del display
y_display=5;

module guias_tapa(xyz)
{
    // Sobre pared derecha - en primer orificio
    translate(xyz + [pared, pos_tornillo_y - (separacion_guias/2) - espesor_guias,0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    translate(xyz + [pared, pos_tornillo_y + (separacion_guias/2),0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    // Sobre pared derecha - en segundo orificio
    translate(xyz + [pared, pos_tornillo_y + separacion_tornillos - (separacion_guias/2) - espesor_guias,0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    translate(xyz + [pared, pos_tornillo_y + separacion_tornillos + (separacion_guias/2),0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    
    // Sobre pared izquierda - en primer orificio
    translate(xyz + [ancho - pared - espesor_guias, pos_tornillo_y - (separacion_guias/2) - espesor_guias,0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    translate(xyz + [ancho - pared - espesor_guias, pos_tornillo_y + (separacion_guias/2),0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    // Sobre pared izquierda - en segundo orificio
    translate(xyz + [ancho - pared - espesor_guias, pos_tornillo_y + separacion_tornillos - (separacion_guias/2) - espesor_guias,0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    translate(xyz + [ancho - pared - espesor_guias, pos_tornillo_y + separacion_tornillos + (separacion_guias/2),0])
        cube([espesor_guias,espesor_guias,profundidad-pared], false);
    
}

module cajon(xyz, x, y, z, p)
{
    difference()
    {
        translate(xyz) cube([x,y,z], false);
        translate(xyz + [pared,pared,pared]) cube([x-(2*pared),y-(2*pared),z], false);
    }
}

module soporte_placa(xyz, h)
{
    translate(xyz) cylinder(h, r = 2.5, $fn = 100);
    translate(xyz+[0,0,h]) cylinder(3, r = 1, $fn = 100);
}

module soportes_rbpi(xyz)
{
    soporte_placa(xyz, h_rbpi);    
    soporte_placa(xyz + [58,0,0], h_rbpi);    
    soporte_placa(xyz + [0,49,0], h_rbpi);    
    soporte_placa(xyz + [58,49,0], h_rbpi);    
    
}

module soportes_dysplay(xyz)
{
    soporte_placa(xyz, h_display);    
    soporte_placa(xyz + [113.11,0,0], h_display);    
    soporte_placa(xyz + [0,85.92,0], h_display);    
    soporte_placa(xyz + [113.11,85.92,0], h_display);    
    
}

module caja(xyz)
{
    cajon(xyz, ancho, alto, profundidad, pared);
    soportes_rbpi(xyz + [x_rbpi,y_rbpi,pared]);
    soportes_dysplay(xyz + [x_display,y_display,pared]);
}

module calado_borde(xyz, b, h)
{
    translate(xyz) cube([b, (pared/2), profundidad_calado], false);
    translate(xyz+[0,h-1,0]) cube([b, (pared/2), profundidad_calado], false);
    translate(xyz) cube([(pared/2), h, profundidad_calado], false);
    translate(xyz+[b-1,0,0]) cube([(pared/2), h, profundidad_calado], false);
    
}

module ventana_lateral(xyz)
{
    // Ventana a 29mm de borde superior y 5 mm de la base, 
    // (3mm sobre el piso de la caja)
    translate(xyz+[ancho-pared,posicion_ventana,5])
        cube([pared, ancho_ventana, profundidad - pared - 3], false);
    
}

module perf_tornillo_tapa(xyz)
{
    translate(xyz)
        rotate(a=[0,90,0])
        cylinder(ancho, r = 1, $fn = 100);

    translate(xyz + [0,separacion_tornillos,0])
        rotate(a=[0,90,0])
        cylinder(ancho, r = 1, $fn = 100);
}


//
// Construcción
//

difference()
{
    caja(inicial);
    // -- menos -->
    calado_borde(inicial + [0,0,profundidad-2], ancho, alto);
    ventana_lateral(inicial);
    perf_tornillo_tapa(inicial + [0,pos_tornillo_y,pos_tornillo_h]);
}

guias_tapa(inicial);






