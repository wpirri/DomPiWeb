<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    if( !isset($PLANTA)) $PLANTA = 1;
    //$objects_json = file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA);
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME."/cgi-bin/abmassign.cgi?funcion=info&Planta=".$PLANTA), true)['response'];
    
    echo "<style>";
    $count = count($obj_list);
    for ($i = 0; $i < $count; $i++)
    {
        //echo "<p>".$obj_list[$i]['Objeto']." - ".$obj_list[$i]['Icono0']."</p>";
        echo "#id-".$obj_list[$i]['Objeto']." {";
            echo "  z-index: 20;";
            echo "  border: 0px;";
            echo "  cursor: pointer;";
            echo "  position: absolute; ";
            echo "  left: ".$obj_list[$i]['Cord_x']."px;";
            echo "  top: ".$obj_list[$i]['Cord_y']."px;";
            echo "}";
    }
    echo "</style>";
?>
