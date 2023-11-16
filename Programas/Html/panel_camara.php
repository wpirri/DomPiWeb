<?php
$TITLE='Tablero de Camaras'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='tablero_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $MAIN?>');" >
	<img id='tablero_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='camera_list' class='abm-cam-list-div'></div>

<div id='camera_image' class='abm-div'></div>

<script type="text/javascript" >
    cam_nombre = '';
    cam_ip = '';
    cam_auth = '';
    cam_proto = '';
    cam_req = '';

    function SelectCamera(nombre, ip, usuario, clave, proto, req) {
      cam_nombre = nombre;
      cam_ip = ip;
      cam_auth = usuario + ':' + clave;
      cam_proto = proto;
      cam_req = req;
    }

    function getCameraImage() {
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
    }

    function refreshImage (img_base64) {
      document.getElementById('camera_image').innerHTML = '<img width=540 src="data:image/png;base64, ' + img_base64 + '" />';
    }

    function loadCameraList (msg) {
      fillCameraShortList(JSON.parse(msg).response, 'camera_list', 'Camaras', 'Id');
      setInterval(getCameraImage, 250);
    }

    function OnLoad() {
      newAJAXCommand('/cgi-bin/abmcamara.cgi?funcion=all', loadCameraList, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
