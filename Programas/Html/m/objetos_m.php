<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>Estados</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
<link href="../css/movil.css" rel="stylesheet" type="text/css" />
<script src="../js/ajax.js" type="text/javascript"></script>
</head>

<?php
    $grupo = $_GET['grupo'];
?>

<body onload="InitUpdate()">

<div class="desktop-group" id="desktop">

<div class="scroll-list" id="event-list">
&nbsp;
</div>

<div class="list-back-btn" id="movil-return" onclick="window.location.replace('index.php');">
	<img id="back-icon" class="icon-image" src="../images/back.png">&nbsp;Volver
</div>

</div>

<script type="text/javascript">

// Parses the xmlResponse from status.xml and updates the status box
function LoadData(msg) {
    // Armo el listado de objetos
	var i = 0;
	var json_list = JSON.parse(msg).response;
    var output = '';
    var filename = '';

    if(!json_list) return;

    // Cabecera
    <?php if($grupo == 1) { ?>
    output += '<div class="list-head" id="list-head1"  >\n';
	output += '<img id="icon-image1" class="icon-image" src="../images/lock.png" >&nbsp;Alarma\n';
    output += '</div>\n';
    <?php } else if($grupo == 2) { ?>
    output += '<div class="list-head" id="list-head2" >\n';
	output += '<img id="icon-image2" class="icon-image" src="../images/lamp1.png" >&nbsp;Luces\n';
    output += '</div>\n';
    <?php } else if($grupo == 3) { ?>
    output += '<div class="list-head"  id="list-head3" >\n';
	output += '<img id="icon-image3" class="icon-image" src="../images/door1.png" >&nbsp;Puertas\n';
    output += '</div>\n';
    <?php } else if($grupo == 4) { ?>
    output += '<div class="list-head"  id="list-head4" >\n';
	output += '<img id="icon-image4" class="icon-image" src="../images/calef1.png" >&nbsp;Clima\n';
    output += '</div>\n';
    <?php } else if($grupo == 4) { ?>
    output += '<div class="list-head"  id="list-head4" >\n';
	output += '<img id="icon-image4" class="icon-image" src="../images/calef1.png" >&nbsp;Clima\n';
    output += '</div>\n';
    <?php } else if($grupo == 5) { ?>
    output += '<div class="list-head"  id="list-head5" >\n';
	output += '<img id="icon-image5" class="icon-image" src="../images/camara.png" >&nbsp;C&aacute;maras\n';
    output += '</div>\n';
    <?php } else if($grupo == 6) { ?>
    output += '<div class="list-head"  id="list-head6" >\n';
	output += '<img id="icon-image6" class="icon-image" src="../images/gear.png" >&nbsp;Riego\n';
    output += '</div>\n';
    <?php } ?>

	for (i = 0; i < json_list.length; i++) {
        if(json_list[i].Estado == 0)
        {
            filename = json_list[i].Icono_Apagado;
        }
        else if(json_list[i].Estado == 1)
        {
            filename = json_list[i].Icono_Encendido;
        }
        else if(json_list[i].Estado == 2)
        {
            filename = json_list[i].Icono_Auto;
        }

        output += '<div class="list-btn-group<?php echo $grupo; ?>" id="list-btn' + i + '" onClick="ChangeStatus(\'' + json_list[i].Objeto + '\');">\n';
        output += '<img id="boton' + i + '" class="icon-image" src="../images/' + filename + '">&nbsp;' + json_list[i].Objeto + '\n';
        output += '</div>\n';
	}    

    document.getElementById('event-list').innerHTML = output;

    newAJAXCommand('/cgi-bin/dompi_mobile.cgi?funcion=list&Grupo=<?php echo $grupo; ?>', UpdateStatus, true);
}

function UpdateStatus(msg) {
    try {
        // actualizo el estado de los objetos
        var i = 0;
        var json_list = JSON.parse(msg).response;
        var filename = '';

        if(!json_list) return;

        for (i = 0; i < json_list.length; i++) {
            if(json_list[i].Estado == 0)
            {
                filename = json_list[i].Icono_Apagado;
            }
            else if(json_list[i].Estado == 1)
            {
                filename = json_list[i].Icono_Encendido;
            }
            else if(json_list[i].Estado == 2)
            {
                filename = json_list[i].Icono_Auto;
            }
            document.getElementById('boton' + i).src = '../images/' + filename;
        }
	} catch (e) { return; }
}

function InitUpdate() {
    newAJAXCommand('/cgi-bin/dompi_mobile.cgi?funcion=list&Grupo=<?php echo $grupo; ?>', LoadData, false);
}

function ChangeStatus(objeto) {
    newAJAXCommand('/cgi-bin/dompi_mobile.cgi?funcion=touch&Objeto=' + objeto, null, false);
}
</script>

</body>
</html>