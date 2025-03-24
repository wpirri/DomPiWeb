<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    $SERVER_PORT = $_SERVER['SERVER_PORT'];

    if( !isset($PLANTA)) $PLANTA = 1;
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME.":".$SERVER_PORT."/cgi-bin/abmassign.cgi?funcion=status&Planta=".$PLANTA), true)['response'];

    ?>
    <script type="text/javascript" >
    var moveObj = false;
    var posX = 0;
    var posY = 0;

    function onMouseDown(id) {
        // 0 : Left button
        // 1 : Wheel or middle button (if present)
        // 2 : Right button
        if(event.button == 0) {
            if(moveObj) {
                if(posX > 0 && posY > 0) {
                    newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=update', null, false, 'Id=' + id + '&Cord_x=' + posX + '&Cord_y=' + posY);
                }
                moveObj = false;
            } else {
                moveObj = true;
            }
        }
    }

    function onMouseUp(js_id) {
        mouseDown = false;
    }

    function onDblClick(obj_id) {
        window.location.replace('edit_assign.php?Id=' + obj_id);
    }

    function onMouseMove(js_id) {
        if(moveObj)
        {
            var e = window.event;
            posX = e.clientX + 40;
            posY = e.clientY - 40;
            obj = document.getElementById(js_id);
            obj.style.left = posX + "px";
            obj.style.top = posY + "px";
        }

    }
    </script>
    <?php

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

        $src = "src=\"images/".$Icono_Encendido."\"";
        $onDblClick = "onDblClick=\"onDblClick('".$Id."');\"";
        $onMouseDown = "onMouseDown=\"onMouseDown('".$Id."');\"";

        $js_id = "id-".$Objeto;
        $onMouseMove = "onMouseMove=\"onMouseMove('".$js_id."');\"";
        $onMouseUp = "onMouseUp=\"onMouseUp('".$js_id."');\"";
    //
        // Representacion del objeto segun el tipo
        //
        if ($Tipo == 2)
        {
	        //echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;9999&nbsp;</div>";
        }
        else if ($Tipo == 6)
        {
			if($Port[0] == 'T')
            {
                // Display de temperatura
                //echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;T 00.0 °C&nbsp;</div>";
			}
            else if($Port[0] == 'H')
            {
                // Display de humedad
                //echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;Hr 00.0 %&nbsp;</div>";
			}
            else
            {
                //echo "<div id=\"id-".$Objeto."\" class=\"home-display\" ".$onclick.">&nbsp;".$Port."&nbsp;</div>";
			}
        }
        else
        {
            // Display generico
	        echo "<img id=\"".$js_id."\" class=\"home-image\" ".$src." ".$onMouseUp." ".$onMouseDown." ".$onDblClick." ".$onMouseMove."/>";
        }
            
    }
?>
