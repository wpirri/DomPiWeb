\u DB_DOMPIWEB

alter table TB_DOM_ASSIGN drop column Icono0;
alter table TB_DOM_ASSIGN drop column Icono1;
alter table TB_DOM_ASSIGN add column Icono_Apagado varchar(32);
alter table TB_DOM_ASSIGN add column Icono_Encendido varchar(32);

update TB_DOM_ASSIGN set Icono_Apagado = 'lamp0.png' where Grupo_Visual = 2;
update TB_DOM_ASSIGN set Icono_Encendido = 'lamp1.png' where Grupo_Visual = 2;
update TB_DOM_ASSIGN set Icono_Apagado = 'door0.png' where Grupo_Visual = 1;
update TB_DOM_ASSIGN set Icono_Encendido = 'door1.png' where Grupo_Visual = 1;
update TB_DOM_ASSIGN set Icono_Apagado = 'key.png' where Grupo_Visual = 3;
update TB_DOM_ASSIGN set Icono_Encendido = 'key.png' where Grupo_Visual = 3;


alter table TB_DOM_AUTO drop column Icono0;
alter table TB_DOM_AUTO drop column Icono1;
alter table TB_DOM_AUTO drop column Icono2;
alter table TB_DOM_AUTO add column Icono_Apagado varchar(32);
alter table TB_DOM_AUTO add column Icono_Encendido varchar(32);
alter table TB_DOM_AUTO add column Icono_Auto varchar(32);


\u DB_DOMPICLOUD

alter table TB_DOMCLOUD_ASSIGN drop column Icono0;
alter table TB_DOMCLOUD_ASSIGN drop column Icono1;
alter table TB_DOMCLOUD_ASSIGN drop column Icono2;
alter table TB_DOMCLOUD_ASSIGN add column Icono_Apagado varchar(32);
alter table TB_DOMCLOUD_ASSIGN add column Icono_Encendido varchar(32);
alter table TB_DOMCLOUD_ASSIGN add column Icono_Auto varchar(32);
