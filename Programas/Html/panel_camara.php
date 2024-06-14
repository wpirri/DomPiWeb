<?php
$TITLE='Tablero de Camaras'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='tablero_list_back_btn' class='back-btn' onclick="window.location.replace('seguridad_menu.php');" >
	<img id='tablero_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='camera_list' class='abm-lateral-list-div'></div>

<div id='camera_image' class='abm-div'></div>
<iframe id='nvr_image' class='abm-div'></iframe>

<script type="text/javascript" >
  <?php
    if( isset( $_GET['cam_nombre'] ) && isset( $_GET['cam_ip'] ) && 
        isset( $_GET['cam_auth'] ) && isset( $_GET['cam_proto'] ) && isset( $_GET['cam_req'] ) )
    {
      ?>
      cam_nombre = '<?php echo $_GET['cam_nombre']; ?>';
      cam_ip = '<?php echo $_GET['cam_ip']; ?>';
      cam_auth = '<?php echo $_GET['cam_auth']; ?>';
      cam_proto = '<?php echo $_GET['cam_proto']; ?>';
      cam_req = '<?php echo $_GET['cam_req']; ?>';
      <?php
    }
    else
    {
      ?>
      cam_nombre = '';
      cam_ip = '';
      cam_auth = '';
      cam_proto = '';
      cam_req = '';
      <?php
    }
  ?>

    function fillCameraShortList(json_list, dst_div) { 
      // Getting the all column names 
      var output = '<table class=abm-list-table>\n';
      var i = 0;
      var j = 0;

      if(json_list.length == 2 && json_list[1]['Protocolo'] == 'iframe')
      {
        document.getElementById('nvr_image').src = json_list[1]['Direccion_IP'];
      }
      else
      {
        for (i = 0; i < json_list.length; i++) {
          if(json_list[i]['Id'] > 0) {
            output += '<tr><td onclick="SelectCamera(';
            output += '\'';
            output += json_list[i]['Nombre'];
            output += '\'';
            output += ',';
            output += '\'';
            output += json_list[i]['Direccion_IP'];
            output += '\'';
            output += ',';
            output += '\'';
            output += json_list[i]['Usuario'];
            output += '\'';
            output += ',';
            output += '\'';
            output += json_list[i]['Clave'];
            output += '\'';
            output += ',';
            output += '\'';
            output += json_list[i]['Protocolo'];
            output += '\'';
            output += ',';
            output += '\'';
            output += json_list[i]['Requerimiento'];
            output += '\'';
            output += ')">';
            output += json_list[i]['Nombre'];
            output += '</td></tr>';
          }
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
      }
    } 

    function SelectCamera(nombre, ip, usuario, clave, proto, req) {
      cam_nombre = nombre;
      cam_ip = ip;
      cam_auth = usuario + ':' + clave;
      cam_proto = proto;
      cam_req = req;
    }

    function getCameraImage() {
      try {
        if(cam_nombre.length > 0)
        {
          if(cam_proto == 'http' || cam_proto == 'https')
          {
            //http://192.168.10.13/tmpfs/auto.jpg&auth=syshome:syshome      
            newAJAXCommand('get_camera_http.php?src=' + cam_proto + '://' + cam_ip + cam_req + '&auth=' + cam_auth, refreshImage, false);
          }
          else if(cam_proto == 'rtsp')
          {
            // rtsp://admin:AUXGCQ@192.168.10.134:554/h264_stream
            newAJAXCommand('get_camera_rtsp.php?src=' + cam_proto + '://' + cam_auth + '@' + cam_ip + cam_req, refreshImage, false);
          }
        }
      } catch(e) {}
      setTimeout(getCameraImage, 1000);
    }

    function refreshImage (img_base64) {
      document.getElementById('camera_image').innerHTML = '<img width=540 src="data:image/png;base64, ' + img_base64 + '" />';
    }

    function loadCameraList (msg) {
      fillCameraShortList(JSON.parse(msg).response, 'camera_list', 'Camaras', 'Id');
      setTimeout(getCameraImage, 1000);
    }

    function OnLoad() {
      newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=all', loadCameraList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
