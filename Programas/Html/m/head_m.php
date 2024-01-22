<!DOCTYPE html>
<html lang="es">
<?php 
    function head_link(string $filename)
    {
        if( ($fm = filemtime($filename)) == false )
        {
            $fm = 0;
        }
        echo "<link href=\"".$filename."?time=".$fm."\" rel=\"stylesheet\" type=\"text/css\" />\n";
    }

    function head_script(string $filename)
    {
        if( ($fm = filemtime($filename)) == false )
        {
            $fm = 0;
        }
        echo "<script src=\"".$filename."?time=".$fm."\" type=\"text/javascript\"></script>\n";
    }

?>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>SYSHOME MOVILE V 1.0 - <?php echo $TITLE; ?></title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
<?php head_link("../css/movil.css"); ?>
<?php head_script("../js/ajax.js"); ?>
</head>
