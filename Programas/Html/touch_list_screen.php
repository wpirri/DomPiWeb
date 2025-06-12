<?php
$TITLE='Tactiles - Pantallas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='touch_list_back_btn' class='back-btn' onclick="window.location.replace('touch_list.php');" >
	<img id='touch_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='touch_list_add_btn' class='add-btn' onclick="AddScreen(<?php echo $_GET['Id']; ?>);" >
	<img id='touch_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nueva
</div>

<div id='touch_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    var max_pantalla;

    function fillTouchList(json_list, dst_div, title, index_label, edit_link) { 
        // Getting the all column names 
        var headers = getAbmTableHedaer(json_list);
        var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-list-table>\n';
        var i = 0;
        var j = 0;
        var index_value = '';

        max_pantalla = (-1)

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
                        if(val > max_pantalla) max_pantalla = val;
                    }
                    // If there is any key, which is matching 
                    // with the column name 
                    if (val == null) val = "&nbsp;";   
                    output += '<td>';
                    output += val;
                    output += '</td>';
                }
            } 
            // Agrego los links de edici�n y borrado
            val = '<td><a href="' + edit_link + '&' +index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
            output += val;
            val = '<td><img src="images/delete.png" onclick="DelScreen(<?php echo $_GET['Id']; ?>,' + index_value + ')" ></td>' 
            output += val;
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function LoadData(msg) {
        fillTouchList(JSON.parse(msg).response, 'touch_list_table_div', '<?php echo $TITLE; ?>', 'Pantalla', 'touch_list_boton.php?Id=<?php echo $_GET['Id']; ?>', 'touch_delete_pantalla.php&Id=<?php echo $_GET['Id']; ?>');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=list&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function AddScreen(Id) {
        document.getElementById('touch_list_table_div').innerHTML = '';
        max_pantalla++;
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=add&Id=' + Id + '&Pantalla=' + max_pantalla + '&Boton=1', LoadData, false);
    }

    function DelScreen(Id, Pantalla) {
        if(Pantalla == max_pantalla) {
            document.getElementById('touch_list_table_div').innerHTML = '';
            newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=delete&Id=' + Id + '&Pantalla=' + Pantalla, LoadData, false);
        }
    }

</script>

</body>

<?php
include('foot.php');
?>
