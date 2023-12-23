<?php
$TITLE='Tablero de Alarmas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='tablero_list_back_btn' class='back-btn' onclick="window.location.replace('seguridad_menu.php');" >
	<img id='tablero_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarm_list' class='abm-lateral-list-div'></div>

<div id='alarm_status' class='abm-div'></div>

<script type="text/javascript" >

    <?php
    if( isset( $_GET['part_nombre']  ) )
    {
      ?>
      part_nombre = '<?php echo $_GET['part_nombre']; ?>';
      <?php
    }
    else
    {
      ?>
      part_nombre = '';
      <?php
    }
    ?>

    function LoadPart() {
        if(part_nombre.length > 0)
        {
            newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=status_part&Nombre=' + part_nombre, fillPartLongList, false);
        }
        else
        {
            setTimeout(LoadPart, 1000);
        }
    }

    function fillPartLongList(msg) { 
      try {
        var i;
        var part = JSON.parse(msg).response;
        var zonas = part.Zonas;
        var salidas = part.Salidas;

        var output = '';
        if(document.getElementById('part-' + part.Nombre)) {

          if( part.Estado_Activacion == 0 ) {
            document.getElementById('Estado_Activacion').src = 'images/lock0.png';
          } else {
            document.getElementById('Estado_Activacion').src = 'images/lock1.png';
          }
          if( part.Estado_Memoria == 0 ) {
            document.getElementById('Estado_Memoria').src = 'images/green.gif';
          } else {
            document.getElementById('Estado_Memoria').src = 'images/red.gif';
          }
          if( part.Estado_Alarma == 0 ) {
            document.getElementById('Estado_Alarma').src = 'images/sirena0.png';
          } else {
            document.getElementById('Estado_Alarma').src = 'images/sirena1.png';
          }
          for (i = 0; i < zonas.length; i++) { 
            if( zonas[i].Activa == 0 ) {
              document.getElementById('zona-' + zonas[i].Objeto).src = 'images/no.png';
            } else if( zonas[i].Estado == 0 ) {
              document.getElementById('zona-' + zonas[i].Objeto).src = 'images/ok.png';
            } else {
              document.getElementById('zona-' + zonas[i].Objeto).src = 'images/no-ok.png';
            }
          }
          for (i = 0; i < salidas.length; i++) { 
            if( salidas[i].Estado == 0 ) {
              document.getElementById('salida-' + salidas[i].Objeto).src = 'images/sirena0.png';
            } else {
              document.getElementById('salida-' + salidas[i].Objeto).src = 'images/sirena1.png';
            }
          }
        } else {
          output = '<p class=abm-table-title>&nbsp;' + part.Nombre + '</p>\n';
          output += '<table class=abm-panel-table id=part-' + part.Nombre + '>\n';
          output += '<tr> <td>Activada:</td> <td>';
          if( part.Estado_Activacion == 0 ) {
            output += '<img id="Estado_Activacion" src="images/lock0.png" />';
          } else {
            output += '<img id="Estado_Activacion" src="images/lock1.png" />';
          }
          output += '</td> <td>&nbsp;</td> </tr>\n';

          output += '<tr> <td>Memoria:</td> <td>';
          if( part.Estado_Memoria == 0 ) {
            output += '<img id="Estado_Memoria" src="images/green.gif" />';
          } else {
            output += '<img id="Estado_Memoria" src="images/red.gif" />';
          }
          output += '</td> <td>&nbsp;</td> </tr>\n';

          output += '<tr> <td>Alarma:</td> <td>';
          if( part.Estado_Alarma == 0 ) {
            output += '<img id="Estado_Alarma" src="images/sirena0.png" />';
          } else {
            output += '<img id="Estado_Alarma" src="images/sirena1.png" />';
          }
          output += '</td> <td>&nbsp;</td> </tr>\n';

          output += '<tr> <td>Zonas</td> <td>&nbsp;</td> <td>&nbsp;</td> </tr>\n';
          for (i = 0; i < zonas.length; i++) { 
            output += '<tr> <td>&nbsp;</td> <td>' + zonas[i].Objeto + '</td> <td>';
            if( zonas[i].Activa == 0 ) {
              output += '<img id="zona-' + zonas[i].Objeto + '" src="images/no.png" />';
            } else if( zonas[i].Estado == 0 ) {
              output += '<img id="zona-' + zonas[i].Objeto + '" src="images/ok.png" />';
            } else {
              output += '<img id="zona-' + zonas[i].Objeto + '" src="images/no-ok.png" />';
            }
            output += '</td> </tr>\n';
          }

          output += '<tr> <td>Salidas</td> <td>&nbsp;</td> <td>&nbsp;</td> </tr>\n';

          for (i = 0; i < salidas.length; i++) { 
            output += '<tr> <td>&nbsp;</td> <td>' + salidas[i].Objeto + '</td> <td>';
            if( salidas[i].Estado == 0 ) {
              output += '<img id="salida-' + salidas[i].Objeto + '" src="images/sirena0.png" />';
            } else {
              output += '<img id="salida-' + salidas[i].Objeto + '" src="images/sirena1.png" />';
            }
            output += '</td> </tr>\n';
          }

          output += '</table>\n';
          document.getElementById('alarm_status').innerHTML = output;
        }

        LoadPart();
      } catch (e) { 
        LoadPart();
        return; 
      }
    }

    function fillPartShortList(json_list, dst_div) { 
      // Getting the all column names 
      var output = '<table class=abm-list-table>\n';
      var i = 0;
      var j = 0;

      // Datos - Salteo el primero de la lista
      for (i = 0; i < json_list.length; i++) {
        if(json_list[i]['Id'] > 0) {
          output += '<tr><td onclick="SelectPart(';
          output += '\'';
          output += json_list[i]['Nombre'];
          output += '\'';
          output += ')">';
          output += json_list[i]['Nombre'];
          output += '</td></tr>';
        }
      }
      output += '</table>\n';
      document.getElementById(dst_div).innerHTML = output;
    } 

    function SelectPart(nombre) {
      part_nombre = nombre;
    }

    function loadPartList (msg) {
      fillPartShortList(JSON.parse(msg).response, 'alarm_list', 'Particiones', 'Id');
      setTimeout(LoadPart, 1000);
    }

    function OnLoad() {
      newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_part', loadPartList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
