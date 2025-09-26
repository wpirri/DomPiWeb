<?php 
    include('config.php'); 
?>
<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>SYSHOME V 3.0</title>
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2021">
<?php head_link("css/index.css"); ?>
<?php head_script("js/ajax.js"); ?>
</head>
<body>
<div id="digital-photo-frame">
  <img id="current-image" src="" alt="Imagen actual">
</div>
<script type="text/javascript">
/* On Click de la página */
window.onclick = function() {window.location.replace('index.php');}

var fotos_files[];
var fotos_cant = 0;
var fotos_index = 0;

function showImage(index) {
  currentImage.src = images[index];
}

function nextImage() {
  currentIndex = (currentIndex + 1) % images.length;
  showImage(currentIndex);
}

function ScanFotos() {
 
}


setInterval(nextImage, 3000); // Cambiar de imagen cada 3 segundos

</script>
</body>
</html>
