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
