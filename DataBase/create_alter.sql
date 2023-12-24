USE DB_DOMPIWEB;

## Previos

ALTER TABLE TB_DOM_CONFIG ADD COLUMN Wifi_AP1 varchar(33);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Wifi_AP1_Pass varchar(65);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Wifi_AP2 varchar(33);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Wifi_AP2_Pass varchar(65);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Wifi_Report integer DEFAULT 0;
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_APN_Auto integer DEFAULT 0;
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_APN varchar(33);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_DNS1 varchar(16);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_DNS2 varchar(16);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_User varchar(17);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_Pass varchar(17);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Gprs_Auth integer DEFAULT 0;		-- 1:PAP 2:CHAP 3:PAP/CHAP
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Send_Method integer DEFAULT 0;	-- 1: First Wifi 2: First GPRS 3: Paralell
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Rqst_Path varchar(256);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Home_Host_1_Address varchar(64);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Home_Host_2_Address varchar(64);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Usuario_Cloud varchar(256);
ALTER TABLE TB_DOM_CONFIG ADD COLUMN Clave_Cloud varchar(256);

ALTER TABLE TB_DOM_ALARM_PARTICION ADD COLUMN Delay_Activacion integer DEFAULT 0;
ALTER TABLE TB_DOM_ALARM_PARTICION ADD COLUMN Delay_Alarma integer DEFAULT 0;


#### Migración de Perif ####
INSERT INTO TB_DOM_PERIF_NEW (Id, MAC, Dispositivo, Tipo, Estado, Direccion_IP, Ultimo_Ok, Actualizar, Flags)
  SELECT Id, MAC, Dispositivo, Tipo, Estado, Direccion_IP, Ultimo_Ok, Actualizar, Flags FROM TB_DOM_PERIF;
ALTER TABLE TB_DOM_PERIF RENAME TO TB_DOM_PERIF_BORRAR;
ALTER TABLE TB_DOM_PERIF_NEW RENAME TO TB_DOM_PERIF;
DROP TABLE TB_DOM_PERIF_BORRAR;
####


## 22/05/2023 - Agregado de índices
## Ejecutar para verificar: sudo mysqltuner.pl
## Ejecutar para optimizar: sudo mysqlcheck --all-databases --optimize
alter table TB_DOM_CONFIG add unique index idx_config_id (Id);
alter table TB_DOM_USER add unique index idx_user_id (Id);
alter table TB_DOM_USER add unique index idx_user_user_pass1 (Usuario,Pin_Teclado);
alter table TB_DOM_USER add unique index idx_user_user_pass2 (Usuario,Pin_SMS);
alter table TB_DOM_USER add unique index idx_user_user_pass3 (Usuario,Pin_WEB);
alter table TB_DOM_USER add index idx_user_tarj (Tarjeta);
alter table TB_DOM_PERIF add unique index idx_perif_id (Id);
alter table TB_DOM_PERIF add unique index idx_perif_mac (MAC);
alter table TB_DOM_GRUPO_VISUAL add UNIQUE INDEX idx_grp_vis_id (Id);
alter table TB_DOM_ASSIGN add unique index idx_assign_id (Id);
alter table TB_DOM_ASSIGN unique add index idx_assign_disp_port (Dispositivo, Port);
alter table TB_DOM_ASSIGN add unique index idx_assign_disp_port_tipo (Dispositivo, Port, Tipo);
alter table TB_DOM_ASSIGN add index idx_assign_disp (Dispositivo);
alter table TB_DOM_ASSIGN add index idx_assign_grupo_visual (Grupo_Visual);
alter table TB_DOM_GROUP add unique index idx_group_id (Id);
alter table TB_DOM_FLAG add unique index idx_flag_id (Id);
alter table TB_DOM_FUNCTION add unique index idx_function_id (Id);
alter table TB_DOM_EVENT add UNIQUE INDEX idx_event_id (Id);
alter table TB_DOM_EVENT add UNIQUE INDEX idx_event_obj_origen (Objeto_Origen);
alter table TB_DOM_AT add UNIQUE INDEX idx_at_id (Id);
alter table TB_DOM_AT add UNIQUE INDEX idx_at_obj_dest (Objeto_Destino);
alter table TB_DOM_AT add UNIQUE INDEX idx_at_fecha (Mes,Dia,Hora,Ultimo_Mes,Ultimo_Dia,Ultima_Hora,Ultimo_Minuto,Dias_Semana);
alter table TB_DOM_ALARM_PARTICION add UNIQUE INDEX idx_ap_id (Id);
alter table TB_DOM_ALARM_PARTICION add UNIQUE INDEX idx_ap_nombre (Nombre);
alter table TB_DOM_ALARM_ZONA add UNIQUE INDEX idx_az_id (Id);
alter table TB_DOM_ALARM_ZONA add INDEX idx_az_part (Particion);
alter table TB_DOM_ALARM_SALIDA add UNIQUE INDEX idx_as_id (Id);
alter table TB_DOM_ALARM_SALIDA add INDEX idx_as_part (Particion);
alter table TB_DOM_AUTO add UNIQUE INDEX idx_auto_id (Id);
alter table TB_DOM_AUTO add INDEX idx_auto_tipo (Tipo);
alter table TB_DOM_AUTO add INDEX idx_auto_obj_sens (Objeto_Sensor);

## 05/07/2023 - Agregado de claves para asistentes
alter table TB_DOM_USER add column Amazon_Key varchar(256);
alter table TB_DOM_USER add column Google_Key varchar(256);
alter table TB_DOM_USER add column Apple_Key varchar(256);
alter table TB_DOM_USER add column Other_Key varchar(256);


alter table TB_DOM_EVENT drop foreign key Funcion_Destino;


