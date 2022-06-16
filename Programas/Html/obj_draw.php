<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    if( !isset($PLANTA)) $PLANTA = 1;
    if( !isset($edit)) $edit = 0;
    //$objects_json = file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA);
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=status&Planta=".$PLANTA), true)['response'];
    
    $count = count($obj_list);
    for ($i = 0; $i < $count; $i++)
    {
        //echo "<p>".$obj_list[$i]['Objeto']." - ".$obj_list[$i]['Icono0']."</p>";
        $Id = $obj_list[$i]['Id'];
        $Icono0 = $obj_list[$i]['Icono0'];
        $Icono1 = $obj_list[$i]['Icono1'];
        $Objeto = str_replace(" ", "-", $obj_list[$i]['Objeto']);
        $Nombre = $obj_list[$i]['Objeto'];
        $Tipo = $obj_list[$i]['Tipo'];
        $Segundos = $obj_list[$i]['Analog_Mult_Div_Valor'];

        if($edit == 1 || $obj_list[$i]['Estado'] == 1)
        {
            $src = "src=\"images/".$Icono1."\"";
        }
        else
        {
            $src = "src=\"images/".$Icono0."\"";
        }

        if($edit == 1)
        {
            $onclick = "onClick=\"window.location.replace('edit_assign.php?Id=".$Id."');\"";
        }
        else if($Tipo == 0)
        {
            // ON / OFF
		    $onclick = "onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=switch&Objeto=".$Nombre."');\"";
        }
        else if($obj_list[$i]['Tipo'] == 5)
        {
            // Pulso
		    $onclick = "onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=pulse&Objeto=".$Nombre."&Segundos=".$Segundos."');\"";
        }
        else
        {
            // Sin control
            $onclick = "";
        }

	    echo "<img id=\"id-".$Objeto."\" class=\"home-image\" ".$src." ".$onclick."/>";
    }
?>
