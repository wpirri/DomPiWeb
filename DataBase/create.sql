
DROP TABLE IF EXISTS TB_DOM_AT;
DROP TABLE IF EXISTS TB_DOM_EVENT_CHANGE;
DROP TABLE IF EXISTS TB_DOM_EVENT;
DROP TABLE IF EXISTS TB_DOM_FUNCTION;
DROP TABLE IF EXISTS TB_DOM_FLAG;
DROP TABLE IF EXISTS TB_DOM_GROUP_LIST;
DROP TABLE IF EXISTS TB_DOM_GROUP;
DROP TABLE IF EXISTS TB_DOM_ASSIGN;
DROP TABLE IF EXISTS TB_DOM_PERIF;
DROP TABLE IF EXISTS TB_DOM_USER;

CREATE TABLE IF NOT EXISTS TB_DOM_USER (
Nombre varchar(16) primary key,         -- NIC Name
Usuario varchar(64) NOT NULL,
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
Id varchar(16) primary key,              -- MAC Address
Dispositivo varchar(128) NOT NULL,
Tipo integer DEFAULT 0,
Estado integer DEFAULT 0,                -- 0=Disable
Config_PORT_A_Analog integer DEFAULT 0,         -- 0=Digital 1=Analogico
Config_PORT_A_E_S integer DEFAULT 255,       -- 0=Output 1=Input
Config_PORT_B_Analog integer DEFAULT 0,         -- 0=Digital 1=Analogico
Config_PORT_B_E_S integer DEFAULT 255,       -- 0=Output 1=Input
Config_PORT_C_Analog integer DEFAULT 0,         -- 0=Digital 1=Analogico
Config_PORT_C_E_S integer DEFAULT 255,       -- 0=Output 1=Input
Direccion_IP varchar(16) DEFAULT "0.0.0.0",
Ultimo_Ok varchar(32),
Estado_PORT_A integer DEFAULT 0,
Estado_PORT_B integer DEFAULT 0,
Estado_PORT_C integer DEFAULT 0,
Estado_Analog_1 integer DEFAULT 0,
Estado_Analog_2 integer DEFAULT 0,
Estado_Analog_3 integer DEFAULT 0,
Estado_Analog_4 integer DEFAULT 0,
Estado_Analog_5 integer DEFAULT 0,
Estado_Analog_6 integer DEFAULT 0,
Estado_Analog_7 integer DEFAULT 0,
Estado_Analog_8 integer DEFAULT 0,
Actualizar integer DEFAULT 0,                -- Enviar update de config al HW
Flags integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_ASSIGN (
Id integer primary key,
Objeto varchar(128) NOT NULL,
Dispositivo varchar(16) NOT NULL,
Port integer NOT NULL,                      -- 1=A, 2=B, 3=C
E_S integer NOT NULL,                       -- Bit 0 a 15
Tipo integer NOT NULL,                      -- 0=Analog, 1=Digital, 2=Alarma
Estado integer DEFAULT 0,
Flags integer DEFAULT 0,
FOREIGN KEY(Dispositivo) REFERENCES TB_DOM_PERIF(Id)
);

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
Enviar integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso a Objto o Grupo
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- ==, >, <
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
Evento integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso a Objto o Grupo
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- ==, >, <
Condicion_Valor integer DEFAULT 0,                -- Valor de condicion
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Destino) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Destino) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Funcion_Destino) REFERENCES TB_DOM_FUNCTION(Id),
FOREIGN KEY(Variable_Destino) REFERENCES TB_DOM_FLAG(Id)
);
