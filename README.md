# DomPiWeb
 Sistema de Domótica con central domiciliaria basada en Raspberry Pi y servicios en la nube para administración externa remota.

 El sistema está formado por una serie de programas en c y c++ que se ejecutan
 dentro del monitor transaccional GMonitor y todo el sistema preparado para
 funcionar en una Raspberry Pi o distribuido entre varias.
 Otro módulo del sistema funciona en un servidor Linux centralizando varios sistemas hogareños permitiendo el acceso remoto a travez de la nube.
 
## Componentes del sistema:
### DomPiWeb: Central domiciliaria de Domótica.
El proyecto contiene distintos programas que se ejecutan como servers dentro de un monitor transaccional GMonitor (https://github.com/wpirri/gmonitor), CGI para hacer de interface con Apache y un micrositio en PHP para la administración local todo este software se ejecuta en una Raspberry Pi. Se incluye además un harware para ser enchufado en el conector de 40 pines de la Raspberry Pi, aprovechar algunos pines de la GPIO y darle alimentación. También se encuentra dentro del proyecto los doseños para imprimir un gabinete para contener la central de domótica y otro para su taería con cargador.

### DomPiIO: Software que permite utilizar la Rasberri Pi como un módulo de expanción de la central de Domótica o como Central de Alarmas integrada al Sistema de Domótica.
El proyecto (https://github.com/wpirri/DomPiIO) contiene distintos programas que se ejecutan como servers dentro de un monitor transaccional GMonitor (https://github.com/wpirri/gmonitor) y CGI para hacer de interface con Apache.

### DomPiClowd: Software que permite controlar los sistemas hogareños desde la nobe.
El proyecto (https://github.com/wpirri/DomPiClowd) contiene distintos programas que se ejecutan como servers dentro de un monitor transaccional GMonitor (https://github.com/wpirri/gmonitor), CGI y un micrositio en PHP para el control de los sistemas de domótica desde un teléfono celular.
