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

  var part_status = 0;
  <?php
  if( isset( $_GET['part_nombre']  ) )
  {
    ?>
    var part_nombre = '<?php echo $_GET['part_nombre']; ?>';
    <?php
  }
  else
  {
    ?>
    var part_nombre = '';
    <?php
  }
  ?>

  function ChangeParticion(part) {
      newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=switch_part&Part=' + part);
  }

  function ChangeZona(part, zona) {
      newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=switch_zona&Part=' + part + '&Zona=' + zona);
  }

  function ChangeSalida(part, salida) {
      newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=pulse_salida&Part=' + part + '&Salida=' + salida);
  }

  function LoadPart() {
      if(part_nombre.length > 0)
      {
          newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=status_part&Part=' + part_nombre, fillPartLongList, false);
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

        part_status = part.Estado_Activacion;
        if( part.Estado_Activacion == 0 ) {
          document.getElementById('Estado_Activacion').src = 'images/lock0.png';
        } else {
          document.getElementById('Estado_Activacion').src = 'images/lock1.png';
        }
        if( part.Estado_Memoria == 0 ) {
          document.getElementById('Estado_Memoria').src = 'images/led_ambar0.png';
          document.getElementById('display_Estado_Memoria').innerHTML = '&nbsp';
        } else {
          document.getElementById('Estado_Memoria').src = 'images/led_ambar1.png';
          document.getElementById('display_Estado_Memoria').innerHTML = part.Estado_Memoria;
        }
        if( part.Estado_Alarma == 0 ) {
          document.getElementById('Estado_Alarma').src = 'images/led_verde0.png';
          document.getElementById('display_Estado_Alarma').innerHTML = '&nbsp;';
        } else {
          document.getElementById('Estado_Alarma').src = 'images/led_verde1.png';
          document.getElementById('display_Estado_Alarma').innerHTML = part.Estado_Alarma + 's';
        }
        for (i = 0; i < zonas.length; i++) { 
          if( zonas[i].Activa == 0 ) {
            document.getElementById('zona-' + zonas[i].Objeto).src = 'images/no.png';
          } else if( zonas[i].Estado == 0 ) {
            document.getElementById('zona-' + zonas[i].Objeto).src = 'images/led_rojo0.png';
          } else {
            document.getElementById('zona-' + zonas[i].Objeto).src = 'images/led_rojo1.png';
          }
        }
        for (i = 0; i < salidas.length; i++) { 
          if( salidas[i].Estado == 0 ) {
            document.getElementById('salida-' + salidas[i].Objeto).src = 'images/sirena_mini0.png';
          } else {
            document.getElementById('salida-' + salidas[i].Objeto).src = 'images/sirena_mini1.png';
          }
        }
      } else {
        output = '<p class=abm-table-title>&nbsp;' + part.Nombre + '</p>\n';
        output += '<table class=abm-panel-table id=part-' + part.Nombre + '>\n';
        output += '<tr> <td>Activada:</td> <td>';
        if( part.Estado_Activacion == 0 ) {
          output += '<img id="Estado_Activacion" class="touch-icon" src="images/lock0.png" onclick="ChangeParticion(\'' + part.Nombre + '\');" />';
        } else {
          output += '<img id="Estado_Activacion"  class="touch-icon" src="images/lock1.png" onclick="ChangeParticion(\'' + part.Nombre + '\');" />';
        }
        output += '</td> <td id="display_Estado_Activacion">&nbsp;</td> </tr>\n';

        output += '<tr> <td>Memoria:</td> <td>';
        if( part.Estado_Memoria == 0 ) {
          output += '<img id="Estado_Memoria" src="images/led_ambar0.png" />';
        } else {
          output += '<img id="Estado_Memoria" src="images/led_ambar1.png" />';
        }
        output += '</td> <td id="display_Estado_Memoria">&nbsp;</td> </tr>\n';

        output += '<tr> <td>Alarma:</td> <td>';
        if( part.Estado_Alarma == 0 ) {
          output += '<img id="Estado_Alarma" src="images/led_verde0.png" />';
        } else {
          output += '<img id="Estado_Alarma" src="images/led_verde1.png" />';
        }
        output += '</td> <td id="display_Estado_Alarma">&nbsp;</td> </tr>\n';

        for (i = 0; i < zonas.length; i++) {
          if(i == 0) {
            output += '<tr> <td>Zonas</td> <td>' + zonas[i].Objeto + '</td> <td>';
          } else {
            output += '<tr> <td>&nbsp;</td> <td>' + zonas[i].Objeto + '</td> <td>';
          }
          if( zonas[i].Activa == 0 ) {
            output += '<img id="zona-' + zonas[i].Objeto + '" src="images/no.png" onClick="ChangeZona(\'' + part.Nombre + '\',\'' + zonas[i].Objeto + '\');" />';
          } else if( zonas[i].Estado == 0 ) {
            output += '<img id="zona-' + zonas[i].Objeto + '" src="images/led_rojo0.png" onClick="ChangeZona(\'' + part.Nombre + '\',\'' + zonas[i].Objeto + '\');" />';
          } else {
            output += '<img id="zona-' + zonas[i].Objeto + '" src="images/led_rojo1.png" onClick="ChangeZona(\'' + part.Nombre + '\',\'' + zonas[i].Objeto + '\');" />';
          }
          output += '</td> </tr>\n';
        }

        for (i = 0; i < salidas.length; i++) { 
          if( i == 0) {
            output += '<tr> <td>Salidas:</td> <td>' + salidas[i].Objeto + '</td> <td>';
          } else {
            output += '<tr> <td>&nbsp;</td> <td>' + salidas[i].Objeto + '</td> <td>';
          }
          if( salidas[i].Estado == 0 ) {
            output += '<img id="salida-' + salidas[i].Objeto + '" src="images/sirena_mini0.png" onClick="ChangeSalida(\'' + part.Nombre + '\',\'' + salidas[i].Objeto + '\');" />';
          } else {
            output += '<img id="salida-' + salidas[i].Objeto + '" src="images/sirena_mini1.png" onClick="ChangeSalida(\'' + part.Nombre + '\',\'' + salidas[i].Objeto + '\');" />';
          }
          output += '</td> </tr>\n';
        }

        output += '</table>\n';
        document.getElementById('alarm_status').innerHTML = output;
      }

      LoadPart();
    } catch (e) { 
      LoadPart();
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
