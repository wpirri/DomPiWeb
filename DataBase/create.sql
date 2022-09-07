
DROP TABLE IF EXISTS TB_DOM_AUTO;
DROP TABLE IF EXISTS TB_DOM_ALARM;
DROP TABLE IF EXISTS TB_DOM_AT;
DROP TABLE IF EXISTS TB_DOM_EVENT_CHANGE;
DROP TABLE IF EXISTS TB_DOM_EVENT;
DROP TABLE IF EXISTS TB_DOM_FUNCTION;
DROP TABLE IF EXISTS TB_DOM_FLAG;
DROP TABLE IF EXISTS TB_DOM_GROUP_LIST;
DROP TABLE IF EXISTS TB_DOM_GROUP;
DROP TABLE IF EXISTS TB_DOM_ASSIGN;
DROP TABLE IF EXISTS TB_DOM_GRUPO_VISUAL;
DROP TABLE IF EXISTS TB_DOM_PERIF;
DROP TABLE IF EXISTS TB_DOM_USER;
DROP TABLE IF EXISTS TB_DOM_CONFIG;

CREATE TABLE IF NOT EXISTS TB_DOM_CONFIG (
Id integer primary key,
Creacion varchar(32),
System_Key varchar(256),
Cloud_Host_1_Address varchar(64),
Cloud_Host_1_Port integer DEFAULT 0,
Cloud_Host_1_Proto varchar(8),
Cloud_Host_2_Address varchar(64),
Cloud_Host_2_Port integer DEFAULT 0,
Cloud_Host_2_Proto varchar(8),
Planta1 varchar(256),
Planta2 varchar(256),
Planta3 varchar(256),
Planta4 varchar(256),
Planta5 varchar(256),
Modem_port varchar(16),
Flags integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_USER (
Id integer primary key,
Usuario varchar(16) NOT NULL,         -- Nombre corto (1 palabra)
Nombre_Completo varchar(64) NOT NULL,        -- Nombre completo
Pin_Teclado varchar(32),
Pin_SMS varchar(32),
Pin_WEB varchar(32),
Telefono_Voz varchar(32),
Telefono_SMS varchar(32),
eMail varchar(64),
Permisos varchar(128),
Dias varchar(8),
Horas varchar(48),
Estado varchar(32), -- Habilitado, Bloqueado [motivo] 
Contador_Error integer DEFAULT 0,
Ultimo_Acceso varchar(32),
Ultimo_Error varchar(32),
Flags integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_PERIF (
Id integer primary key,
MAC varchar(16) NOT NULL,                       -- MAC Address
Dispositivo varchar(128) NOT NULL,
Tipo integer DEFAULT 0,                         -- 0=RBPi, 1=Wifi HTTP
Estado integer DEFAULT 0,                       -- 0=Disable
Direccion_IP varchar(16) DEFAULT "0.0.0.0",
Ultimo_Ok varchar(32),
Actualizar integer DEFAULT 0,                   -- Enviar update de config al HW
Flags integer DEFAULT 0
);

CREATE TABLE TB_DOM_GRUPO_VISUAL (
Id integer primary key,
Nombre varchar(32) NOT NULL,
Descripcion varchar(256),
Icono varchar(32)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ASSIGN (
Id integer primary key,
Objeto varchar(128) NOT NULL,               -- Nombre para identificarlo en el sistema
Dispositivo integer NOT NULL,               -- Discpositivo - Id de TB_DOM_PERIF
Port varchar(128) NOT NULL,                 -- Nombre con el que se identifica en el dispositivo
Tipo integer NOT NULL,                      -- 0=Output, 1=Input, 2=Analog, 3=Output Alarma, 4=Input Alarma, 5=Output Pulse/Analog_Mult_Div_Valor=Pulse Param
Estado integer DEFAULT 0,                   -- 1 / 0 para digitales 0 a n para analogicos
Estado_HW integer DEFAULT 0,                -- Estado reportado por el HW
Icono0 varchar(32),
Icono1 varchar(32),
Grupo_Visual integer DEFAULT 0,
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Coeficiente integer DEFAULT 0,              -- 1=Coeficiente Positivo, -1=Coeficiente Negativo  - rc = Coeficiente * ( (Analog_Mult_Div)?Estado/Analog_Mult_Div_Valor:Estado*Analog_Mult_Div_Valor ) 
Analog_Mult_Div integer DEFAULT 0,          -- 0=Multiplicar por valor, 1=Dividir por valor
Analog_Mult_Div_Valor integer DEFAULT 1,
Actualizar integer DEFAULT 0,                   -- Enviar update de config al HW por este PORT
Flags integer DEFAULT 0,
FOREIGN KEY(Dispositivo) REFERENCES TB_DOM_PERIF(Id)
FOREIGN KEY(Grupo_Visual) REFERENCES TB_DOM_GRUPO_VISUAL(Id)
);

# ALTER TABLE TB_DOM_ASSIGN ADD COLUMN Estado_HW integer DEFAULT 0; 

CREATE TABLE IF NOT EXISTS TB_DOM_GROUP ( 
Id integer primary key,
Grupo varchar(128) NOT NULL
);

CREATE TABLE IF NOT EXISTS TB_DOM_GROUP_LIST (
Grupo integer primary key,
Objeto integer NOT NULL,
Principal integer DEFAULT 0,            -- Define el estado del resto del grupo
Invertir integer DEFAULT 0,             -- Mantiene estado invertido con respecto al grupo
FOREIGN KEY(Grupo) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Objeto) REFERENCES TB_DOM_ASSIGN(Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_FLAG (
Id integer primary key,
Variable varchar(128) NOT NULL,
Valor integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_FUNCTION (
Id integer primary key,
Funcion varchar(128) NOT NULL
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT (
Id integer primary key,
Evento varchar(128) NOT NULL,
Objeto_Origen integer NOT NULL,
Objeto_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Grupo_Destino integer NOT NULL,         -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Funcion_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Variable_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
ON_a_OFF integer DEFAULT 0,
OFF_a_ON integer DEFAULT 0,
Enviar integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Switch 4=Pulso a Objeto o Grupo. Si no Variable = Enviar
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- 0 ==, 1 >, 2 <
Condicion_Valor integer DEFAULT 0,                -- Valor de condicion
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Origen) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Objeto_Destino) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Destino) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Funcion_Destino) REFERENCES TB_DOM_FUNCTION(Id),
FOREIGN KEY(Variable_Destino) REFERENCES TB_DOM_FLAG(Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_AT (
Id integer primary key,
Agenda varchar(128) NOT NULL,
Mes integer DEFAULT 0,
Dia integer DEFAULT 0,
Hora integer DEFAULT 0,
Minuto integer DEFAULT 0,
Dias_Semana varchar(128),
Objeto_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Grupo_Destino integer NOT NULL,         -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Funcion_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Variable_Destino integer NOT NULL,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Evento integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Switch 4=Pulso a Objeto o Grupo. Si no Variable = Enviar
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- ==, >, <
Condicion_Valor integer DEFAULT 0,                -- Valor de condicion
Ultima_Ejecucion varchar(32),
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Destino) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Destino) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Funcion_Destino) REFERENCES TB_DOM_FUNCTION(Id),
FOREIGN KEY(Variable_Destino) REFERENCES TB_DOM_FLAG(Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ALARM (
Part integer primary key,
ActStatus integer DEFAULT 0,
MemStatus integer DEFAULT 0,
InZone1 integer DEFAULT 0,
InZone2 integer DEFAULT 0,
InZone3 integer DEFAULT 0,
InZone4 integer DEFAULT 0,
InZone5 integer DEFAULT 0,
InZone6 integer DEFAULT 0,
InZone7 integer DEFAULT 0,
InZone8 integer DEFAULT 0,
InActDes integer DEFAULT 0,
OutSiren integer DEFAULT 0,
OutBuzer integer DEFAULT 0,
FOREIGN KEY(InZone1) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone2) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone3) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone4) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone5) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone6) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone7) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InZone8) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(InActDes) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(OutSiren) REFERENCES TB_DOM_ASSIGN(Id)
FOREIGN KEY(OutBuzer) REFERENCES TB_DOM_ASSIGN(Id)
);

-- Sistema de riego
-- Calefaccion
-- Aire acondicionado
CREATE TABLE IF NOT EXISTS TB_DOM_AUTO (
Id integer primary key,
Objeto varchar(128) NOT NULL,               -- Nombre para identificarlo en el sistema
Tipo integer default 0,                     -- 0 = Riego 1 = Calefaccion 2 = Aire acondicionado
Objeto_Salida integer NOT NULL,               -- Discpositivo - Id de TB_DOM_ASSIGN
Objeto_Sensor integer NOT NULL,               -- Discpositivo - Id de TB_DOM_ASSIGN
Min_Sensor integer DEFAULT 0,
Max_Sensor integer DEFAULT 0,
Hora_Inicio integer DEFAULT 0,
Hora_Fin integer DEFAULT 0,
Dias_Semana integer DEFAULT 0,
Estado integer DEFAULT 0,                   -- 0 = Disable 1 = Enable
Icono0 varchar(32),
Icono1 varchar(32),
Grupo_Visual integer DEFAULT 0,
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Actualizar integer DEFAULT 0,
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Salida) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Objeto_Sensor) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Visual) REFERENCES TB_DOM_GRUPO_VISUAL(Id)
);
