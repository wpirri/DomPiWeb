
DROP TABLE IF EXISTS TB_DOM_AT;
DROP TABLE IF EXISTS TB_DOM_EVENT_CHANGE;
DROP TABLE IF EXISTS TB_DOM_EVENT_OFF;
DROP TABLE IF EXISTS TB_DOM_EVENT_ON;
DROP TABLE IF EXISTS TB_DOM_EVENT;
DROP TABLE IF EXISTS TB_DOM_FUNCTION;
DROP TABLE IF EXISTS TB_DOM_FLAG;
DROP TABLE IF EXISTS TB_DOM_GROUP_LIST;
DROP TABLE IF EXISTS TB_DOM_GROUP;
DROP TABLE IF EXISTS TB_DOM_ASSIGN;
DROP TABLE IF EXISTS TB_DOM_PERIF;
DROP TABLE IF EXISTS TB_DOM_USER;

CREATE TABLE IF NOT EXISTS TB_DOM_USER (
user_id varchar(16) primary key,         -- NIC Name
user_name varchar(64) NOT NULL,
pin_keypad varchar(32),
pin_sms varchar(32),
pin_web varchar(32),
phone_call varchar(32),
phone_sms varchar(32),
email varchar(64),
access_mask varchar(128),
days_of_week varchar(8),
hours_of_day varchar(32),
user_status varchar(32), -- Habilitado, Bloqueado [motivo] 
access_error_count integer DEFAULT 0,
last_access_ok varchar(32),
last_access_error varchar(32),
user_flags integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_PERIF (
hw_id varchar(16) primary key,              -- MAC Address
name varchar(128) NOT NULL,
hw_typ integer DEFAULT 0,
hw_status integer DEFAULT 0,                -- 0=Disable
config_porta_ana integer DEFAULT 0,         -- 0=Digital 1=Analogico
config_porta_io	 integer DEFAULT 255,       -- 0=Output 1=Input
config_portb_ana integer DEFAULT 0,
config_portb_io	 integer DEFAULT 255,
config_portc_ana integer DEFAULT 0,
config_portc_io	 integer DEFAULT 255,
ip_address varchar(16) DEFAULT "0.0.0.0",
last_ok DATETIME DEFAULT 0,                  -- UNIX Time
status_porta integer DEFAULT 0,
status_portb integer DEFAULT 0,
status_portc integer DEFAULT 0,
status_analog1 integer DEFAULT 0,
status_analog2 integer DEFAULT 0,
status_analog3 integer DEFAULT 0,
status_analog4 integer DEFAULT 0,
status_analog5 integer DEFAULT 0,
status_analog6 integer DEFAULT 0,
status_analog7 integer DEFAULT 0,
status_analog8 integer DEFAULT 0,
hw_update integer DEFAULT 0,                -- Enviar update de config al HW
hw_flags integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_ASSIGN (
ass_id integer primary key,
hw_id varchar(16) NOT NULL,
port_id integer NOT NULL,
io_id integer NOT NULL,
io_typ integer NOT NULL,
name varchar(128) NOT NULL,
ass_status integer DEFAULT 0,
ass_flags integer DEFAULT 0,
FOREIGN KEY(hw_id) REFERENCES TB_DOM_PERIF(hw_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_GROUP ( 
grp_id integer primary key,
name varchar(128) NOT NULL
);

CREATE TABLE IF NOT EXISTS TB_DOM_GROUP_LIST (
grp_id integer primary key,
ass_id integer NOT NULL,
principal integer DEFAULT 0,            -- Define el estado del resto del grupo
invertir integer DEFAULT 0,             -- Mantiene estado invertido con respecto al grupo
FOREIGN KEY(grp_id) REFERENCES TB_DOM_GROUP(grp_id),
FOREIGN KEY(ass_id) REFERENCES TB_DOM_ASSIGN(ass_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_FLAG (
flag_id integer primary key,
name varchar(128) NOT NULL,
flag_value integer DEFAULT 0
);

CREATE TABLE IF NOT EXISTS TB_DOM_FUNCTION (
fcn_id integer primary key,
name varchar(128) NOT NULL
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT (
ev_id integer primary key,
src_ass_id integer NOT NULL,
name varchar(128) NOT NULL,
FOREIGN KEY(src_ass_id) REFERENCES TB_DOM_ASSIGN(ass_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT_ON (
ev_id integer primary key,
dst_ass_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_grp_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_fcn_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_flag_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
snd_ev integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso
snd_ev_param1 integer DEFAULT 0,
if_id integer DEFAULT 0,                -- Condicion
if_val integer DEFAULT 0,               -- Valor de condicion
ev_flags integer DEFAULT 0,
FOREIGN KEY(dst_ass_id) REFERENCES TB_DOM_ASSIGN(ass_id),
FOREIGN KEY(dst_grp_id) REFERENCES TB_DOM_GROUP(grp_id),
FOREIGN KEY(dst_fcn_id) REFERENCES TB_DOM_FUNCTION(fcn_id),
FOREIGN KEY(dst_flag_id) REFERENCES TB_DOM_FLAG(flag_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT_OFF (
ev_id integer primary key,
dst_ass_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_grp_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_fcn_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_flag_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
snd_ev integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso
snd_ev_param1 integer DEFAULT 0,
if_id integer DEFAULT 0,                -- Condicion
if_val integer DEFAULT 0,               -- Valor de condicion
ev_flags integer DEFAULT 0,
FOREIGN KEY(dst_ass_id) REFERENCES TB_DOM_ASSIGN(ass_id),
FOREIGN KEY(dst_grp_id) REFERENCES TB_DOM_GROUP(grp_id),
FOREIGN KEY(dst_fcn_id) REFERENCES TB_DOM_FUNCTION(fcn_id),
FOREIGN KEY(dst_flag_id) REFERENCES TB_DOM_FLAG(flag_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT_CHANGE (
ev_id integer primary key,
dst_ass_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_grp_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_fcn_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
dst_flag_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
snd_ev integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso
snd_ev_param1 integer DEFAULT 0,
if_id integer DEFAULT 0,                -- Condicion
if_val integer DEFAULT 0,               -- Valor de condicion
ev_flags integer DEFAULT 0,
FOREIGN KEY(dst_ass_id) REFERENCES TB_DOM_ASSIGN(ass_id),
FOREIGN KEY(dst_grp_id) REFERENCES TB_DOM_GROUP(grp_id),
FOREIGN KEY(dst_fcn_id) REFERENCES TB_DOM_FUNCTION(fcn_id),
FOREIGN KEY(dst_flag_id) REFERENCES TB_DOM_FLAG(flag_id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_AT (
at_id integer primary key,
name varchar(128) NOT NULL,
at_mon integer DEFAULT 0,
at_day integer DEFAULT 0,
at_hour integer DEFAULT 0,
at_min integer DEFAULT 0,
at_wdays integer DEFAULT 0,
dst_ass_id integer DEFAULT 0,         -- Solo uno de los trs assign ,grupo o funcion
dst_grp_id integer DEFAULT 0,         -- Solo uno de los trs assign ,grupo o funcion
dst_fcn_id integer DEFAULT 0,           -- Solo uno de los trs assign ,grupo o funcion
dst_flag_id integer NOT NULL,         -- Solo uno de los dos assign o grupo
snd_ev integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Pulso
snd_ev_param1 integer DEFAULT 0,
if_id integer DEFAULT 0,                -- Condicion
if_val integer DEFAULT 0,               -- Valor de condicion
ev_flags integer DEFAULT 0,
FOREIGN KEY(dst_ass_id) REFERENCES TB_DOM_ASSIGN(ass_id),
FOREIGN KEY(dst_grp_id) REFERENCES TB_DOM_GROUP(grp_id),
FOREIGN KEY(dst_fcn_id) REFERENCES TB_DOM_FUNCTION(fcn_id),
FOREIGN KEY(dst_flag_id) REFERENCES TB_DOM_FLAG(flag_id)
);
