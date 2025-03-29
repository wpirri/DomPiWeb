<?php
    $REQUEST_SCHEME = $_SERVER['REQUEST_SCHEME'];
    $SERVER_NAME = $_SERVER['SERVER_NAME'];
    $SERVER_PORT = $_SERVER['SERVER_PORT'];

    if( !isset($PLANTA)) $PLANTA = 1;
    $obj_list = json_decode(file_get_contents($REQUEST_SCHEME."://".$SERVER_NAME.":".$SERVER_PORT."/cgi-bin/abmassign.cgi?funcion=status&Planta=".$PLANTA), true)['response'];

    ?>
    <script type="text/javascript" >
    var moveObj = false;
    var delta_x = 0;
    var delta_y = 0;
    var mouse_pos_x = 0;
    var mouse_pos_y = 0;
    var pic_pos_x = 0;
    var pic_pos_y = 0;
    var pic_obj = null;

    function onMouseDown(obj_id, js_id) {
        // 0 : Left button
        // 1 : Wheel or middle button (if present)
        // 2 : Right button
        if(event.button == 0) {
            if(moveObj) {
                if(pic_pos_x > 0 && pic_pos_y > 0) {
                    newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=update', null, false, 'Id=' + obj_id + '&Cord_x=' + pic_pos_x + '&Cord_y=' + pic_pos_y);
                }
                moveObj = false;
                pic_pos_x = 0;
                pic_pos_y = 0;
            } else {
                moveObj = true;
                pic_obj = document.getElementById(js_id);
                pic_pos_x = pic_obj.offsetLeft;
                pic_pos_y = pic_obj.offsetTop;
                delta_x = 0; 
                delta_y = 0;
                //alert('pic x: ' + pic_pos.x + ' y: ' + pic_pos.y + '\n css x: ' + pic_obj.offsetLeft + ' y: ' + pic_obj.offsetTop);
            }
        }
    }

    function onMouseUp(obj_id, js_id) {
        mouseDown = false;
    }

    function onDblClick(obj_id, js_id) {
        window.location.replace('edit_assign.php?Id=' + obj_id);
    }

    function onMouseMove(obj_id, js_id) {
        if(moveObj) {
            var e = window.event;
            if(delta_x == 0 && delta_y == 0) {
                delta_x = pic_pos_x - e.clientX;
                delta_y = pic_pos_y - e.clientY;
            }
            pic_pos_x = e.clientX + delta_x;
            pic_pos_y = e.clientY + delta_y;
            //alert('pic x: ' + pic_pos.x + ' y: ' + pic_pos.y + '\n css x: ' + pic_obj.offsetLeft + ' y: ' + pic_obj.offsetTop);
            pic_obj.style.left = pic_pos_x + "px";
            pic_obj.style.top = pic_pos_y + "px";
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
        $js_id = "id-".$Objeto;
        $onDblClick = "onDblClick=\"onDblClick('".$Id."','".$js_id."');\"";
        $onMouseDown = "onMouseDown=\"onMouseDown('".$Id."','".$js_id."');\"";
        $onMouseMove = "onMouseMove=\"onMouseMove('".$Id."','".$js_id."');\"";
        $onMouseUp = "onMouseUp=\"onMouseUp('".$Id."','".$js_id."');\"";
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
