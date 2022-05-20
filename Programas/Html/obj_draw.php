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
        if($edit == 1)
        {
		    echo "<img id=\"id-".$obj_list[$i]['Objeto']."\" class=\"home-image\" src=\"images/".$obj_list[$i]['Icono1']."\" onClick=\"window.location.replace('edit_assign.php?Id=".$obj_list[$i]['Id']."');\"/>";
        }
        else if($obj_list[$i]['Estado'] == 1)
        {
		    echo "<img id=\"id-".$obj_list[$i]['Objeto']."\" class=\"home-image\" src=\"images/".$obj_list[$i]['Icono1']."\" onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=switch&Objeto=".$obj_list[$i]['Objeto']."');\"/>";
        }
        else
        {
		    echo "<img id=\"id-".$obj_list[$i]['Objeto']."\" class=\"home-image\" src=\"images/".$obj_list[$i]['Icono0']."\" onClick=\"newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=switch&Objeto=".$obj_list[$i]['Objeto']."');\"/>";
        }
    }
?>
