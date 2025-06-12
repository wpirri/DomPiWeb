<?php
$TITLE='Tactiles - Botones'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='touch_list_back_btn' class='back-btn' onclick="window.location.replace('touch_list_screen.php?Id=<?php echo $_GET['Id']; ?>');" >
	<img id='touch_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='touch_list_add_btn' class='add-btn' onclick="AddBoton(<?php echo $_GET['Id']; ?>,<?php echo $_GET['Pantalla']; ?>);" >
	<img id='touch_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='touch_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    var max_boton;

    function fillTouchList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
        // Getting the all column names 
        var headers = getAbmTableHedaer(json_list);
        var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-list-table>\n';
        var i = 0;
        var j = 0;
        var index_value = '';

        max_boton = 0;
        // Header
        output += '<tr>';
        for (i = 0; i < headers.length; i++) { 
            if(headers[i] != index_label) {
                output += '<th>';
                output += headers[i];
                output += '</th>';
            }
        }
        // Agrego las columnas de edici�n y borrado
        output += '<th>Editar</th>';
        output += '<th>Borrar</th>';
        output += '</tr>\n';
        // Datos - Salteo el primero de la lista
        for (i = 0; i < json_list.length; i++) {
            output += '<tr>';
            index_value = '';
            for (j = 0; j < headers.length; j++) { 
                var val = json_list[i][headers[j]]; 
                if(headers[j] == index_label) {
                    index_value = val;
                } else {
                    if(headers[j] == "Nro") {
                        if(val > max_boton) max_boton = val;
                    }
                    // If there is any key, which is matching 
                    // with the column name 
                    if (val == null) val = "&nbsp;";   
                    output += '<td>';
                    if(headers[j] == "Evento")
                    {
                        output += ListaEventoTouch[val].label;
                    }
                    else
                    {
                        output += val;
                    }
                    output += '</td>';
                }
            } 
            // Agrego los links de edicion y borrado
            val = '<td><a href="' + edit_link + '&' +index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
            output += val;
            val = '<td><img src="images/delete.png" onclick="DelBoton(<?php echo $_GET['Id']; ?>,<?php echo $_GET['Pantalla']; ?>,' + index_value + ')" ></td>' 
            output += val;
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function LoadData(msg) {
        fillTouchList(JSON.parse(msg).response, 'touch_list_table_div', '<?php echo $TITLE; ?>', 'Boton', 'touch_edit_boton.php?Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>', 'touch_delete_pantalla.php&Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=list&Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>', LoadData, false);
    }

    function AddBoton(Id, Pantalla) {
        document.getElementById('touch_list_table_div').innerHTML = '';
        max_boton++;
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=add&Id=' + Id + '&Pantalla=' + Pantalla + '&Boton=' + max_boton, LoadData, false);
    }

    function DelBoton(Id, Pantalla, Boton) {
        if(Boton == max_boton && Boton > 1) {
            document.getElementById('touch_list_table_div').innerHTML = '';
            newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=delete&Id=' + Id + '&Pantalla=' + Pantalla + '&Boton=' + Boton, LoadData, false);
        }
    }

</script>

</body>

<?php
include('foot.php');
?>
