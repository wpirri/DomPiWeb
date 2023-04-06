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
$fs = 1;

// Medidas de la caja y tapa
ancho=170;
alto=200;
profundidad=70;
pared=2;
ancho_bateria = 155;
ancho_fuente = 115;
alto_fuente = 82;

// Detalles de la caja (se usan tambien en la Tapa)
profundidad_calado=2;   // Calado superior en los bodes

// Detalles de la Tapa (Se usan también en la caja)
traba_ancho=5;
traba_alto=13;
traba_pestana=1;
traba_distancia_vertical=20;
orificio_cables = 10;


//
// Caja
//

module entrada_cables()
{
    translate(inicial + [0, alto-alto_fuente+orificio_cables, profundidad/2]) rotate([0,90, 0]) cylinder(h=pared, d=orificio_cables, center=false);
}


module tornillo_fuente()
{
    diametro_orificio = 3.5;
    h_orificio1 = 25;
    h_orificio2 = 52;

    pos_orificio_i = 65;
    pos_orificio_d = 70;
    diff_orificios = pos_orificio_d - pos_orificio_i;
    
    tope_derecho_fuente = (ancho / 2) - (ancho_fuente / 2);
    
    translate(inicial + [tope_derecho_fuente + pos_orificio_i, (alto - alto_fuente + h_orificio1), 0])
        cylinder(h = pared, d = diametro_orificio, center = false);
    translate(inicial + [tope_derecho_fuente + pos_orificio_i, (alto - alto_fuente + h_orificio1 - (diametro_orificio/2)), 0])
        cube([diff_orificios,diametro_orificio,pared]);
    translate(inicial + [tope_derecho_fuente + pos_orificio_d, (alto - alto_fuente + h_orificio1), 0])
        cylinder(h = pared, d = diametro_orificio, center = false);
    
    translate(inicial + [tope_derecho_fuente + pos_orificio_i, (alto - alto_fuente + h_orificio2), 0])
        cylinder(h = pared, d = diametro_orificio, center = false);
    translate(inicial + [tope_derecho_fuente + pos_orificio_i, (alto - alto_fuente + h_orificio2 - (diametro_orificio/2)), 0])
        cube([diff_orificios,diametro_orificio,pared]);
    translate(inicial + [tope_derecho_fuente + pos_orificio_d, (alto - alto_fuente + h_orificio2), 0])
        cylinder(h = pared, d = diametro_orificio, center = false);
}

module seccion_fuente()
{
    tope_fuente_x1 = (ancho / 2) - (ancho_fuente / 2);
    tope_fuente_x2 = (ancho / 2) + (ancho_fuente / 2);
    tope_fuente = 3;

    translate(inicial + [tope_fuente_x1, (alto-pared-alto_fuente), pared]) cube([2,alto_fuente,tope_fuente]);
    translate(inicial + [tope_fuente_x2, (alto-pared-alto_fuente), pared]) cube([2,alto_fuente,tope_fuente]);

    translate(inicial + [tope_fuente_x1, (alto-pared-alto_fuente), pared]) cube([ancho_fuente,2,tope_fuente]);
    
}

module tope_bateria()
{
    tope_bateria_x1 = (ancho / 2) - (ancho_bateria / 2);
    tope_bateria_x2 = (ancho / 2) + (ancho_bateria / 2);
    translate(inicial + [tope_bateria_x1, pared, pared]) cube([2,5,(profundidad/2)]);
    translate(inicial + [tope_bateria_x2, pared, pared]) cube([2,5,(profundidad/2)]);
}

module orificio_fijacion(xyz)
{
    translate(xyz + [2.5,2.5,0]) cylinder(h=pared, d=5, center=false);
    translate(xyz - [0,2.5,0]) cube([5,5,pared], center=false);
    translate(xyz + [2.5,0,0] - [0,5,0]) cylinder(h=pared, d=10, center=false);
}

module ventilacion_z(xyz, largo)
{
    translate(xyz) cube([pared,2,largo]);
    translate(xyz + [0,5,0]) cube([pared,2,largo]);
    translate(xyz + [0,10,0]) cube([pared,2,largo]);
    translate(xyz + [0,15,0]) cube([pared,2,largo]);
    translate(xyz + [0,20,0]) cube([pared,2,largo]);
}

module cajon(xyz, x, y, z, p)
{
    tope_bateria();
    seccion_fuente();
    difference()
    {
        translate(xyz) cube([x,y,z], false);
        translate(xyz + [p,p,p]) cube([x-(2*p),y-(2*p),z], false);
    }
}

module calado_borde(xyz, b, h)
{
    translate(xyz) cube([b, (pared/2), profundidad_calado], false);
    translate(xyz+[0,h-1,0]) cube([b, (pared/2), profundidad_calado], false);
    translate(xyz) cube([(pared/2), h, profundidad_calado], false);
    translate(xyz+[b-1,0,0]) cube([(pared/2), h, profundidad_calado], false);
    
}

module orificio_traba(xyz, dir)
{
    if(dir == 1)
    {
        translate(xyz-[0,(traba_ancho*0.125),0]+[0,0,(pared*0.125)]) cube([pared,(traba_ancho*1.25),(pared*1.25)], false);
    }
    else
    {
        translate(xyz-[pared,(traba_ancho*0.125),0]+[0,0,(pared*0.125)]) cube([pared,(traba_ancho*1.25),(pared*1.25)], false);
    }
}

module Caja()
{
    difference()
    {
        cajon(inicial, ancho, alto, profundidad, pared);
        // -- menos -->
        calado_borde(inicial + [0,0,profundidad-2], ancho, alto);
        orificio_traba([0,traba_distancia_vertical,profundidad-traba_alto-pared], 1);
        orificio_traba([0,alto-traba_distancia_vertical-traba_ancho,profundidad-traba_alto-pared], 1);
        
        orificio_traba([ancho,traba_distancia_vertical,profundidad-traba_alto-pared], 2);
        orificio_traba([ancho,alto-traba_distancia_vertical-traba_ancho,profundidad-traba_alto-pared], 2);
        
        ventilacion_z([0,20,(profundidad/4)-10], (profundidad/2));
        ventilacion_z([0,(alto-40),(profundidad/4)-10], (profundidad/2));
        
        ventilacion_z([(ancho-pared),20,(profundidad/4)-10], (profundidad/2));
        ventilacion_z([(ancho-pared),(alto-40),(profundidad/4)-10], (profundidad/2));
        
        orificio_fijacion([((ancho/2)-(ancho_fuente/2)-10),alto-40,0]);
        orificio_fijacion([((ancho/2)+(ancho_fuente/2)+10),alto-40,0]);
        orificio_fijacion([ancho/2,40,0]);

        entrada_cables();
        
        tornillo_fuente();
    }
}

//
// Tapa
//

module ventilacion_x(xyz, largo)
{
    pos = 0;
    for(i = [0:5:largo])
    {
        translate(xyz + [i,0,0]) cube([2,20,pared]);
    }
}

module traba(xyz, dir)
{
    if(dir == 1)
    {
        translate(xyz) cube([pared,traba_ancho,traba_alto], false);
        translate(xyz + [-(traba_pestana),0,traba_alto]) cube([(traba_pestana+pared),traba_ancho,pared], false);
    }
    else
    {
        translate(xyz - [pared,0,0]) cube([pared,traba_ancho,traba_alto], false);
        translate(xyz - [pared,0,0]+ [0,0,traba_alto]) cube([(traba_pestana+pared),traba_ancho,pared], false);
    }
}

module bordes(xyz, b, h)
{
    translate(xyz) cube([b, (pared/2), profundidad_calado], false);
    translate(xyz) cube([(pared/2), h, profundidad_calado], false);
    translate(xyz + [b-(pared/2),0,0]) cube([(pared/2), h, profundidad_calado], false);
    translate(xyz + [0,h-(pared/2),0]) cube([b, (pared/2), profundidad_calado], false);
}

module frente_tapa()
{
    translate(inicial) cube([ancho, alto, pared], false);

    bordes(inicial + [0,0,pared], ancho, alto);

    traba(inicial + [pared, traba_distancia_vertical,pared], 1);
    traba(inicial + [pared, alto-traba_distancia_vertical-traba_ancho,pared], 1);
    traba(inicial + [ancho-pared, traba_distancia_vertical,pared], 2);
    traba(inicial + [ancho-pared, alto-traba_distancia_vertical-traba_ancho,pared], 2);

}

module Tapa()
{
    difference()
    {
        frente_tapa();

        ventilacion_x([(ancho/4),20,0], (ancho/2));
        ventilacion_x([(ancho/4),(alto-40),0], (ancho/2));
    }
}

//
// Construcción
//


Caja();
//Tapa();
