//////////////////////////////////////////////////////////////////////////////
//
//  cam.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1

//  Include files
#include <uf.h>
#include <uf_exit.h>
#include <uf_ui.h>

/*
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
# include <strstream>
  using std::ostrstream;
  using std::endl;
  using std::ends;
#else
# include <strstream.h>
#endif
#include <iostream.h>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uf_obj.h>
#include <uf_ui_ont.h>
#include <uf_oper.h>
#include <uf_part.h>
#include <uf_ugopenint.h>
#include <uf_param.h>


#include "opu_copy_param.h"

#define UF_CALL(X) (report( __FILE__, __LINE__, #X, (X)))

static int report( char *file, int line, char *call, int irc)
{
  if (irc)
  {
     char    messg[133];
     printf("%s, line %d:  %s\n", file, line, call);
     (UF_get_fail_message(irc, messg)) ?
       printf("    returned a %d\n", irc) :
       printf("    returned error %d:  %s\n", irc, messg);
  }
  return(irc);
}

char *grip_exe = "opu_copy_param.grx";
char grip_exe_path[256];

/*----------------------------------------------------------------------------*/
int _run_grip_init ( )
{
 int i , ready , status ;
 char *path , envpath[255] , dllpath[255] ;

 char env_names[][25]={
        "UGII_USER_DIR" ,
        "UGII_SITE_DIR" ,
        "UGII_VENDOR_DIR" ,
        "UGII_GROUP_DIR" ,
        "USER_UFUN" ,
        "UGII_INITIAL_UFUN_DIR" ,
        "UGII_INITIAL_GRIP_DIR" ,
        "UGII_TMP_DIR" ,
        "HOME" ,
        "TMP" } ;

 path = (char *) malloc(255+50);

 ready=-1;
  for (i=0;i<10;i++) {
    dllpath[0]='\0';
    envpath[0]='\0';
    path=envpath;
    UF_translate_variable(env_names[i], &path);
    if (path!=NULL) {
       strcpy(dllpath,path);
       strcat(dllpath,"\\application\\");
       strcat(dllpath,grip_exe);
       UF_print_syslog(dllpath,FALSE);
       // работа с файлом
       UF_CFI_ask_file_exist (dllpath, &status );
       if (!status) { ready=1; break ; }
     } //if (envpath!=NULL) {
  } // for

 if (ready==-1) {
    printf ("Файл %s - не существует \n ...",grip_exe);
    uc1601("GRIP Файл не существует ",1);
    return (-1);
 }

 grip_exe_path[0]='\0';
 sprintf(grip_exe_path,"%s",dllpath);

 return (0);
}

/*----------------------------------------------------------------------------*/
int _run_grip ( char *from_name ,  char *name)
{
    double       user_response = 0;
    int          status = -1;
    int          grip_arg_count = 3;
    UF_args_t    grip_arg_list[3];

  /* Define the argument list for UG/Open API calling GRIP */
    grip_arg_list[0].type    = UF_TYPE_CHAR;
    grip_arg_list[0].length  = 0;
    grip_arg_list[0].address = from_name ;
    grip_arg_list[1].type    = UF_TYPE_CHAR;
    grip_arg_list[1].length  = 0;
    grip_arg_list[1].address = name ;
    grip_arg_list[2].type    = UF_TYPE_DOUBLE;
    grip_arg_list[2].length  = 0;
    grip_arg_list[2].address = &user_response;

    UF_CFI_ask_file_exist (grip_exe_path, &status );
    if (status) { return (-1); }

 /* Execute the GRIP program */
    status = UF_CALL(  UF_call_grip (grip_exe_path, grip_arg_count, grip_arg_list) );
    printf("\n\tUF_call_grip=%d user_response=%.0f",status,user_response);

  if ( status != 0 ) { user_response=-1; }
  if ( (status == 0) && (user_response == 0) ) { ; }

  return ((int)user_response);
}


int opu_copy_param()
{
/*  Variable Declarations */
    char str[133];

    int   i , count = 0 ;
    int   obj_count1 = 0, obj_count2 = 0;
    tag_t *tags1 = NULL , *tags2 = NULL ;
    tag_t prg1 = NULL_TAG,prg2 = NULL_TAG;
    //UF_OPER_OPNAME_LEN UF_OPER_MAX_NAME_LEN
    char  prog_name0[UF_OPER_MAX_NAME_LEN+1] ;
    char  prog_name1[UF_OPER_OPNAME_LEN],prog_name2[UF_OPER_OPNAME_LEN];
    int   type1, subtype1 ;
    int   type2, subtype2 ;
    char  menu[14][38];
    int response ;
    logical logresp ;
    int ret ;

    /************************************************************************/
    char *mes1[]={
      "Программа предназначена для изменения параметров в операциях.",
      "Проводит инициализацию параметров операций из заданной операции.",
          " при использовании данной программы:",
          "   рекомендуем предварительно сохранить ранее сделанные изменения.",
      NULL
    };
    UF_UI_message_buttons_t buttons1 = { TRUE, FALSE, TRUE, "Далее....", NULL, "Отмена", 1, 0, 2 };

    response=0;
    UF_UI_message_dialog("Cam.....", UF_UI_MESSAGE_INFORMATION, mes1, 4, TRUE, &buttons1, &response);
    if (response!=1) { return (0) ; }

    //if (0<_run_grip_init()) { return (-1) ; }
    if (0<_run_grip_init()) {
       UF_UI_display_nonmodal_msg("COPY PARAM","Вы можете использовать только метод копирования параметров",UF_UI_MSG_POS_TOP_LEFT);
    }

    int module_id;
    UF_ask_application_module(&module_id);
    if (UF_APP_CAM!=module_id) {
       // UF_APP_GATEWAY UF_APP_CAM UF_APP_MODELING UF_APP_NONE
       uc1601("Запуск DLL - производится из модуля обработки\n- 2005г.",1);
       return (-2);
    }

    /* Ask displayed part tag */
    if (NULL_TAG==UF_PART_ask_display_part()) {
      uc1601("Cam-часть не активна.....\n Операция прервана.",1);
      return (-3);
    }

  /************************************************************************/

  strcpy(&menu[0][0], "Step 1. Fix 1\0");
  strcpy(&menu[1][0], "Step 2. Fix N\0");
  strcpy(&menu[2][0], "Step 3.1 Reinit (ALL)\0");
  strcpy(&menu[3][0], "Step 3.2 Reinit (only parameters)\0");
  strcpy(&menu[4][0], "Help\0");

  response=3;
  while (response != 1 && response != 2  && response != 19 )
  {

   response = uc1603(".Выберите операцию(-и) ->",4,menu, 4+1);

   printf("\n\nopu_copy_param()\n\tuc1603()=%d",response);

   if (response >= 3 || response < 19 )
   {
       switch (response)
       {
        case 5 :
          if (obj_count1>0) { obj_count1=0 ; UF_free(tags1); }
          UF_CALL( UF_UI_ONT_ask_selected_nodes(&obj_count1, &tags1) );
          printf("\n\t UF_UI_ONT_ask_selected_nodes() obj_count1=%d",obj_count1);
          if (obj_count1<=0) { uc1601("Не выбрана основная операция!\n ..",1); break ; }
          if (obj_count1>1) { uc1601("Необходимо выбрать только 1 операцию!\n ..",1); break ; }
          prg1=tags1[0];
          UF_CALL( UF_OBJ_ask_type_and_subtype (prg1, &type1, &subtype1 ) );
          if (type1!=UF_machining_operation_type) { uc1601("Выберите Операцию\n .процесс прерван.",1); break ; }
        break ;
        case 6 :
          if (obj_count2>0) { obj_count2=0 ; UF_free(tags2); }
          UF_CALL( UF_UI_ONT_ask_selected_nodes(&obj_count2, &tags2) );
          printf("\n\t UF_UI_ONT_ask_selected_nodes() obj_count2=%d",obj_count2);
          if (obj_count2<=0) { uc1601("Не выбрано операций для установки параметров!\n ..",1); break ; }
        break ;
        case 7 :case 8 :
          if (obj_count1<=0) { uc1601("Не выбрана основная операция!\n.процесс прерван.",1); break ; }
          if (obj_count1>1) { uc1601("Необходимо выбрать только 1 основную операцию!\n.процесс прерван.",1); break ; }
          if (obj_count2<=0) { uc1601("Не выбрано операций для установки параметров!\n.процесс прерван.",1); break ; }
          UF_UI_toggle_stoplight(1);
          prg1=tags1[0];
          UF_CALL( UF_OBJ_ask_type_and_subtype (prg1, &type1, &subtype1 ) );
          if (type1!=UF_machining_operation_type) { uc1601("Выберите Основную Операцию\n .процесс прерван.",1); break ; }

          prog_name0[0]='\0';
          UF_CALL( UF_OPER_ask_name_from_tag(prg1, prog_name0) );
          prog_name1[0]='\0'; sprintf(prog_name1,"%.30s",prog_name0);
          printf("\n\tname1=%s, type1=%d, subtype1=%d ",prog_name1,type1, subtype1);
          for(i=0,count=0;i<obj_count2;i++)
          {
            prg2 = tags2[i]; // идентификатор объекта
            //UF_CALL( UF_OPER_ask_oper_type(prg2, &subtype2) );
            //printf("\n\tUF_OPER_ask_oper_type= subtype2=%d ",subtype2);
            UF_CALL( UF_OBJ_ask_type_and_subtype (prg2, &type2, &subtype2 ) );
            if (type2!=UF_machining_operation_type) { continue ; }

            prog_name0[0]='\0';
            //UF_OBJ_ask_name(prg, prog_name0);// спросим имя обьекта
            UF_CALL( UF_OPER_ask_name_from_tag(prg2, prog_name0) );
            prog_name2[0]='\0'; sprintf(prog_name2,"%.30s",prog_name0);
            printf("\n\tname2=%s, type2=%d, subtype2=%d ",prog_name2,type2, subtype2);

            if (prg1==prg2) {
                str[0]='\0';
                sprintf(str,"Имя исходной операции (%s)\n совпадает с назначенным для иниц. (%s)",prog_name1 , prog_name2);
                uc1601(str,1);
                continue ;
            }

            if (subtype1!=subtype2) {
                UF_UI_is_listing_window_open(&logresp);
                if (logresp==TRUE) UF_UI_open_listing_window();
                UF_UI_write_listing_window("\n#------------ Stub ----------------");
                str[0]='\0'; sprintf(str,"\n\t Исходная операция=%s (подтип=%d)",prog_name1,subtype1);
                UF_UI_write_listing_window(str);
                str[0]='\0'; sprintf(str,"\n\t Инициализируемая операция=%s (подтип=%d)",prog_name2,subtype2);
                UF_UI_write_listing_window(str);
                UF_UI_write_listing_window("\n  - подтипы операций не совпадают!!! инициализация прервана..");
                UF_UI_write_listing_window("\n#------------ Stub ----------------");
                continue ;
            }

            if (response==7)
             ret=_run_grip ( prog_name1 , prog_name2); // all
            else
             ret=UF_CALL( UF_PARAM_reinit(prg2,prg1) );
             //UF_PARAM_duplicate( tag_t  old_obj_tag, const char   *name, tag_t        *new_obj_tag );

            if (ret==0) count++;

          }

          UF_UI_ONT_refresh();
          UF_UI_toggle_stoplight(0);
          str[0]='\0'; sprintf(str,"\nПереинициализировано =%d операций.\n$$",count);
          UF_UI_is_listing_window_open(&logresp);
          if (logresp==TRUE) UF_UI_write_listing_window(str);
          uc1601(str,1);
        break ;
        case 9 :
            UF_UI_open_listing_window();
            UF_UI_write_listing_window("\n#============================================================");
            UF_UI_write_listing_window("\n Программа предназначена для инициализации параметров из заданной");
            UF_UI_write_listing_window("\n (основной) операции/программы.  ");
            UF_UI_write_listing_window("\n Происходит передача всех параметров, кроме геометрии.");
            UF_UI_write_listing_window("\n Важно!!! при использовании данной программы:");
            UF_UI_write_listing_window("\n          рекомендуем предварительно сохранить ранее сделанные изменения.");
            UF_UI_write_listing_window("\n Автор:");
            UF_UI_write_listing_window("\n\t Че - 2005г.");
            UF_UI_write_listing_window("\n#============================================================");
            UF_UI_write_listing_window("\n как пользоваться:");
            UF_UI_write_listing_window("\n Шаг 1: выберите основную операцию, с которой вы хотите снять все параметры,\n\t- нажмите кнопку меню 'Step 1. Fix 1';");
            UF_UI_write_listing_window("\n Шаг 2: выберите операции, на которые Вы хотите передать параметры,\n\t- нажмите кнопку меню 'Step 2. Fix N'");
            UF_UI_write_listing_window("\n Шаг 3: Если Вы хотите инициализировать Ваши операции:");
            UF_UI_write_listing_window("\n          параметрами, геометрией, инструментом - нажмите 'Step 3.1 Reinit (ALL)'");
            UF_UI_write_listing_window("\n        Если только необходимы параметры        - нажмите 'Step 3.2 Reinit (only parameters)'");
            UF_UI_write_listing_window("\n#============================================================");
            UF_UI_write_listing_window("\n Замечание:");
            UF_UI_write_listing_window("\n Инициализация будет происходить только если операции имеют");
            UF_UI_write_listing_window("\n  одинаковый подтип - mill_planar-mill_planar");
            UF_UI_write_listing_window("\n                    - mill_contour-mill_contour ... и т.д.");
            UF_UI_write_listing_window("\n#============================================================\n$$");
          //UF_UI_close_listing_window () ;
        break ;
        default : break ;
        }
   }

  } // end while uc1603

 if (obj_count1>0) UF_free(tags1);
 if (obj_count2>0) UF_free(tags2);

 UF_DISP_refresh ();

 printf("\n\nopu_copy_param() - end");

 return (0);
}



//----------------------------------------------------------------------------
//  Activation Methods
//----------------------------------------------------------------------------

//  Explicit Activation
//      This entry point is used to activate the application explicitly, as in
//      "File->Execute UG/Open->User Function..."
extern "C" DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    int errorCode = UF_initialize();

    if ( 0 == errorCode )
    {
        /* TODO: Add your application code here */
        opu_copy_param();

        /* Terminate the API environment */
        errorCode = UF_terminate();
    }

    /* Print out any error messages */
    PrintErrorMessage( errorCode );
}

//----------------------------------------------------------------------------
//  Utilities
//----------------------------------------------------------------------------

// Unload Handler
//     This function specifies when to unload your application from Unigraphics.
//     If your application registers a callback (from a MenuScript item or a
//     User Defined Object for example), this function MUST return
//     "UF_UNLOAD_UG_TERMINATE".
extern "C" int ufusr_ask_unload( void )
{
     /* unload immediately after application exits*/
    return ( UF_UNLOAD_IMMEDIATELY );

     /*via the unload selection dialog... */
     //return ( UF_UNLOAD_SEL_DIALOG );
     /*when UG terminates...              */
     //return ( UF_UNLOAD_UG_TERMINATE );
}

/* PrintErrorMessage
**
**     Prints error messages to standard error and the Unigraphics status
**     line. */
static void PrintErrorMessage( int errorCode )
{
    if ( 0 != errorCode )
    {
        /* Retrieve the associated error message */
        char message[133];
        UF_get_fail_message( errorCode, message );

        /* Print out the message */
        UF_UI_set_status( message );

        fprintf( stderr, "%s\n", message );
    }
}
