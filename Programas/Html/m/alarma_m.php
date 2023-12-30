<?php
$TITLE='Alarma'; 
include("m_head.php");
?>

<?php
    $part = $_GET['part'];
?>

<body onload="OnLoad()">

<div class="desktop-group" id="desktop">

<div class="scroll-list" id="event-list">
&nbsp;
</div>

<div class="list-back-btn" id="movil-return" onclick="window.location.replace('particiones_m.php');">
	<img id="back-icon" class="icon-image" src="../images/back.png">&nbsp;Volver
</div>

</div>

<script type="text/javascript">

    var particion_nombre = '';
    var part_status = 0;

// Parses the xmlResponse from status.xml and updates the status box
function LoadData(msg) {
    // Armo el listado de objetos
	var i = 0;
    var part = JSON.parse(msg).response;
    if(!part) return;
    var zonas = part.Zonas;
    var salidas = part.Salidas;
    var output = '';

    // Cabecera
    if(document.getElementById('icon-alarma'))
    {
        if(part.Estado_Activacion > 0) {
            document.getElementById('icon-alarma').src = '../images/lock1.png';
            part_status = 1;
        } else {
            document.getElementById('icon-alarma').src = '../images/lock0.png';
            part_status = 0;
        }
         
        for (i = 0; i < zonas.length; i++) {
            if(zonas[i].Activa > 0) {
                if (zonas[i].Estado > 0) {
                    document.getElementById('icon-zona' + i).src = '../images/led_rojo1.png';
                } else {
                    document.getElementById('icon-zona' + i).src = '../images/led_rojo0.png';
                }
            } else {
                document.getElementById('icon-zona' + i).src = '../images/no.png';
            }
        }    

        for (i = 0; i < salidas.length; i++) {
            if (salidas[i].Estado > 0) {
                document.getElementById('icon-salida' + i).src = '../images/sirena_mini1.png';
            } else {
                document.getElementById('icon-salida' + i).src = '../images/sirena_mini0.png';
            }
        }    

    }
    else
    {
        output += '<div class="list-head" onClick="ChangeParticion();" >\n';
        output += '<img id="icon-alarma" class="icon-image" src="../images/';
        if(part.Estado_Activacion > 0) {
            output += 'lock1.png';
            part_status = 1;
        } else {
            output += 'lock0.png';
            part_status = 0;
        }
        output += '" />&nbsp;' + part.Nombre + '\n';
        output += '</div>\n';

        for (i = 0; i < zonas.length; i++) {
            output += '<div class="list-btn-group1" onClick="ChangeZona(\'' + zonas[i].Objeto + '\');">\n';
            output += '<img id="icon-zona' + i + '" src="../images/'
            if(zonas[i].Activa > 0) {
                if (zonas[i].Estado > 0) {
                    output += 'led_rojo1.png';
                } else {
                    output += 'led_rojo0.png';
                }
            } else {
                output += 'images/no.png';
            }
            output += '" >&nbsp;' + zonas[i].Objeto + '\n';
            output += '</div>\n';
        }    

        for (i = 0; i < salidas.length; i++) {
            output += '<div class="list-btn-group1" onClick="ChangeSalida(\'' + salidas[i].Objeto + '\');">\n';
            output += '<img id="icon-salida' + i + '" src="../images/'
            if (salidas[i].Estado > 0) {
                output += 'sirena_mini1.png';
            } else {
                output += 'sirena_mini0.png';
            }
            output += '">&nbsp;' + salidas[i].Objeto + '\n';
            output += '</div>\n';
        }    

        document.getElementById('event-list').innerHTML = output;
    }
}

function OnLoad() {
    particion_nombre = '<?php echo $part; ?>';
    newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=status_part&Nombre=<?php echo $part; ?>', LoadData, true);
}

function ChangeParticion() {
    if(part_status > 0) {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=off_part&Nombre=' + particion_nombre, false, false);
    }
    else {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=on_total_part&Nombre=' + particion_nombre, false, false);
    }
}

function ChangeZona() {

}

function ChangeSalida() {

}

</script>

</body>
<?php
include("m_foot.php");
?>
