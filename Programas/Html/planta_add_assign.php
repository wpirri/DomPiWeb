<?php
$TITLE='Agregar Objeto'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='ass_list_back_btn' class='back-btn' onclick="window.location.replace('planta_edit.php');" >
	<img id='ass_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='ass_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    // Pantalla de edición para agregar objetos al map
    function fillAddAssignList(json_list, dst_div, title, add_fcn) { 
        // Getting the all column names 
        var headers = getAbmTableHedaer(json_list);
        var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-list-table>\n';
        var i = 0;
        var j = 0;
        var index_value = '';

        // Header
        output += '<tr>';
        for (i = 0; i < headers.length; i++) { 
            if(headers[i] != 'Id') {
                output += '<th>';
                output += headers[i];
                output += '</th>';
            }
        }
        // Agrego la columna de add
        output += '<th>Agregar</th>';
        output += '</tr>\n';
        // Datos
        for (i = 0; i < json_list.length; i++) {
            if(json_list[i]['Id'] > 0) {
                output += '<tr>';
                index_value = '';
                for (j = 0; j < headers.length; j++) { 
                    var val = json_list[i][headers[j]]; 
                    // If there is any key, which is matching 
                    // with the column name 
                    if (val == null) val = "&nbsp;";   
                    if(headers[j] == 'Id') {
                        index_value = val;
                    }
                    else
                    {
                        output += '<td>';
                        output += val;
                        output += '</td>';
                    }
                } 
                // Agrego el link de add
                val = '<td><img src="images/edit.png" OnClick="' + add_fcn + '(' + index_value + ');"></a></td>' 
                output += val;
                output += '</tr>\n';
            }
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function LoadData(msg) {
        fillAddAssignList(JSON.parse(msg).response, 'ass_list_table_div', '<?php echo $TITLE; ?>', 'AddAssignToPlanta');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmassign.cgi', LoadData, false);
    }

    function AddAssignToPlanta(id) {
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=addassigntoplanta&Id=' + id, null, false);
        setTimeout("window.location.replace('planta_edit.php');", 1000);
    }
</script>

</body>

<?php
include('foot.php');
?>
