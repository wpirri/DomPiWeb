<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    $SERVER_PORT = $_SERVER['SERVER_PORT'];
    
    if( !isset($PLANTA)) $PLANTA = 1;
    //$objects_json = file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA);
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME.":".$SERVER_PORT."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA), true)['response'];
    
    echo "<style>";
    $count = count($obj_list);
    for ($i = 0; $i < $count; $i++)
    {
        // Sustituciones (Mantener: obj_style.php objdraw.php planta.php)
        $Objeto = $obj_list[$i]['Objeto'];
        $Objeto = str_replace(" ", "", $Objeto);
        $Objeto = str_replace(".", "", $Objeto);

        $Cord_x = $obj_list[$i]['Cord_x'];
        $Cord_y = $obj_list[$i]['Cord_y'];
        //echo "<p>".$obj_list[$i]['Objeto']." - ".$obj_list[$i]['Icono_Apagado']."</p>";
        echo "#id-".$Objeto." {";
            echo "  z-index: 20;";
            echo "  border: 0px;";
            echo "  cursor: pointer;";
            echo "  position: absolute; ";
            echo "  left: ".$Cord_x."px;";
            echo "  top: ".$Cord_y."px;";
            echo "}";
    }
    echo "</style>";
?>
