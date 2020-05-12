opu_copy_param

Purpose:
  Программа копирует (переинициализирует) параметры 
  из заданной операции (шаблона)..... другой операции.

===================================================================================

1) Согласно GRIP команде:

 OPER/REINIT,'operation_name',
         FROM{,'from_oper_name'|'template_type','template_subtype'}[,IFERR,label:]

Description
 This command allows you to re-initialize operation parameters or post commands.
 This is not available for Sequential Surface or Drive Curve Lathe.


2) а также функцией  UF_PARAM_reinit(dest,from)

===================================================================================

Requirements for this package:
  UG V17/V18/NX1/NX2/....
  WindowsNT/2000/XP
  UG/Gateway
  UG/Manufacturing
  UG/Grip
  UG/Open API Execute

Restrictions:
  not
