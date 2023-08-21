<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    $SERVER_PORT = $_SERVER['SERVER_PORT'];

    if( !isset($PLANTA)) $PLANTA = 1;
    if( !isset($edit)) $edit = 0;
    //$objects_json = file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA);
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME.":".$SERVER_PORT."/cgi-bin/abmassign.cgi?funcion=status&Planta=".$PLANTA), true)['response'];
    
    $count = count($obj_list);
    for ($i = 0; $i < $count; $i++)
    {
        //echo "<p>".$obj_list[$i]['Objeto']." - ".$obj_list[$i]['Icono_Apagado']."</p>";
        $Id = $obj_list[$i]['Id'];
        $Port = $obj_list[$i]['Port'];
        $Icono_Apagado = $obj_list[$i]['Icono_Apagado'];
        $Icono_Encendido = $obj_list[$i]['Icono_Encendido'];

        // Sustituciones (Mantener: obj_style.php objdraw.php planta.php)
        $Objeto = $obj_list[$i]['Objeto'];
        $Objeto = str_replace(" ", "", $Objeto);
        $Objeto = str_replace(".", "", $Objeto);

        $Nombre = $obj_list[$i]['Objeto'];
        $Tipo = $obj_list[$i]['Tipo'];
        $Segundos = $obj_list[$i]['Analog_Mult_Div_Valor'];

        //
        // Icono segun el estado
        //
        if($edit == 1 || $obj_list[$i]['Estado'] == 1)
        {
            $src = "src=\"images/".$Icono_Encendido."\"";
        }
        else
        {
            $src = "src=\"images/".$Icono_Apagado."\"";
        }

        //
        //  Acciones de click sobre objeto
        //
        if($edit == 1)
        {
            $onclick = "onClick=\"window.location.replace('edit_assign.php?Id=".$Id."');\"";
        }
        else if($Tipo == 0)
        {
            // ON / OFF
		    $onclick = "onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=switch&Objeto=".$Nombre."');\"";
        }
        else if($Tipo == 5)
        {
            // Pulso
		    $onclick = "onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=pulse&Objeto=".$Nombre."&Segundos=".$Segundos."');\"";
        }
        else
        {
            // Sin control
            $onclick = "";
        }

        //
        // Representacion del objeto segun el tipo
        //
        if ($Tipo == 2)
        {
	        echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;9999&nbsp;</div>";
        }
        else if ($Tipo == 6)
        {
			if($Port[0] == 'T')
            {
                // Display de temperatura
                echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;T 00.0 °C&nbsp;</div>";
			}
            else if($Port[0] == 'H')
            {
                // Display de humedad
                echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;Hr 00.0 %&nbsp;</div>";
			}
            else
            {
                echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;".$Port."&nbsp;</div>";
			}
        }
        else
        {
            // Display generico
	        echo "<img id=\"id-".$Objeto."\" class=\"home-image\" ".$src." ".$onclick."/>";
        }
            
    }
?>
