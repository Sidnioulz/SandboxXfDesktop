/*   edit.c */

/*  Copyright (C)  Jean-Fran�ois Wauthy under GNU GPL
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "menueditor.h"

#include "../modules/menu/dummy_icon.h"

#include "add_menu_dialog.h"
#include "edit_dialog.h"

void editmenu_option_file_cb (GtkWidget *widget, struct _controls_menu *controls)
{
  controls->menu_type = MENUFILE;
  gtk_widget_set_sensitive(controls->hbox_source,TRUE);
  gtk_widget_set_sensitive(controls->label_source,TRUE);
  gtk_widget_set_sensitive(controls->label_style,FALSE);
  gtk_widget_set_sensitive(controls->optionmenu_style,FALSE);
  gtk_widget_set_sensitive(controls->checkbutton_unique,FALSE);
}

void editmenu_option_system_cb (GtkWidget *widget, struct _controls_menu *controls)
{
  controls->menu_type = SYSTEM;
  gtk_widget_set_sensitive(controls->hbox_source,FALSE);
  gtk_widget_set_sensitive(controls->label_source,FALSE);
  gtk_widget_set_sensitive(controls->label_style,TRUE);
  gtk_widget_set_sensitive(controls->optionmenu_style,TRUE);
  gtk_widget_set_sensitive(controls->checkbutton_unique,TRUE);
}

void edit_selection(GtkTreeSelection *selection)
{
  xmlNodePtr node;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GValue val = { 0, };

  GtkWidget *dialog;
  GtkWidget *header;
  GtkWidget *header_image;
  gchar *header_text;

  GtkWidget* name_entry = NULL;
  GtkWidget* command_entry = NULL;
  GtkWidget *icon_entry = NULL;
  struct _controls_menu controls;
  GtkWidget *entry_source = NULL;
  GtkWidget *checkbutton_snotify = NULL;
  GtkWidget *checkbutton_term = NULL;

  xmlChar *prop_name = NULL;
  xmlChar *prop_cmd = NULL;
  xmlChar *prop_icon = NULL;
  xmlChar *prop_type = NULL;
  xmlChar *prop_src = NULL;
  xmlChar *prop_snotify = NULL;
  xmlChar *prop_term = NULL;
  xmlChar *prop_style = NULL;
  xmlChar *prop_unique = NULL;

  /* Retrieve the xmlNodePtr of the menu entry */
  gtk_tree_selection_get_selected (selection, &model, &iter);
	
  gtk_tree_model_get_value (GTK_TREE_MODEL(menueditor_app.treestore), &iter, POINTER_COLUMN, &val);
  node = g_value_get_pointer(&val);

  if(!node)
    return;

  /* Create dialog for editing */
  dialog = gtk_dialog_new_with_buttons(_("Edit menu entry"),
				       GTK_WINDOW(menueditor_app.main_window),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_STOCK_CANCEL,
				       GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,
				       GTK_RESPONSE_OK,NULL);

  /* set ok button as default */
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

  /* Header */
  header_image = gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_LARGE_TOOLBAR);
  header_text = g_strdup_printf("%s", _("Edit menu entry"));
  header = xfce_create_header_with_image (header_image, header_text);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), header, FALSE, FALSE, 0);
  g_free (header_text);

  prop_name = xmlGetProp(node, "name");
  prop_cmd = xmlGetProp(node, "cmd");
  prop_icon = xmlGetProp(node, "icon");
  prop_type = xmlGetProp(node, "type");
  prop_src = xmlGetProp(node, "src");
  prop_snotify = xmlGetProp(node , "snotify");
  prop_term = xmlGetProp(node, "term");
  prop_unique = xmlGetProp(node, "unique");
  prop_style = xmlGetProp(node, "style");

  /* Choose the edition dialog */
  if(!xmlStrcmp(node->name,(xmlChar*)"separator") ){
    gtk_widget_destroy (dialog);
    return;
  }else if(!xmlStrcmp(node->name,(xmlChar*)"include")){
    GtkWidget *mitem;

    GtkWidget *table;
    GtkWidget *label_type;
    GtkWidget *menu;
    GtkWidget *optionmenu_type;

    GtkWidget *button_browse;

    /* Table */
    table = gtk_table_new(2, 2, TRUE);

    /* Type */
    label_type = gtk_label_new(_("Type:"));

    menu = gtk_menu_new();
    mitem = gtk_menu_item_new_with_mnemonic(_("File"));
    g_signal_connect ((gpointer) mitem, "activate", G_CALLBACK (editmenu_option_file_cb), &controls);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    gtk_widget_show(mitem);

    mitem = gtk_menu_item_new_with_mnemonic(_("System"));
    g_signal_connect ((gpointer) mitem, "activate", G_CALLBACK (editmenu_option_system_cb), &controls);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    gtk_widget_show(mitem);

    optionmenu_type = gtk_option_menu_new();

    gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu_type), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_type), 0);

    gtk_table_attach(GTK_TABLE(table), label_type, 0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), optionmenu_type, 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);

    /* Source */
    controls.hbox_source = gtk_hbox_new(FALSE, 0);
    controls.label_source = gtk_label_new(_("Source:"));
    entry_source = gtk_entry_new();
    button_browse = gtk_button_new_with_label("...");

    g_signal_connect ((gpointer) button_browse, "clicked", G_CALLBACK (browse_command_cb), entry_source);

    gtk_box_pack_start (GTK_BOX (controls.hbox_source), entry_source, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (controls.hbox_source), button_browse, FALSE, FALSE, 0);

    gtk_table_attach(GTK_TABLE(table), controls.label_source, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), controls.hbox_source, 1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

    /* Style */
    controls.label_style = gtk_label_new(_("Style:"));
  
    menu = gtk_menu_new();
    mitem = gtk_menu_item_new_with_mnemonic(_("Simple"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    gtk_widget_show(mitem);
    mitem = gtk_menu_item_new_with_mnemonic(_("Multilevel"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
    gtk_widget_show(mitem);

    controls.optionmenu_style = gtk_option_menu_new();

    gtk_option_menu_set_menu(GTK_OPTION_MENU(controls.optionmenu_style), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(controls.optionmenu_style), 0);
  
    gtk_table_attach(GTK_TABLE(table), controls.label_style, 0, 1, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), controls.optionmenu_style, 1, 2, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);

    /* Unique */
    controls.checkbutton_unique = gtk_check_button_new_with_mnemonic(_("_Unique entries only"));

    gtk_table_attach(GTK_TABLE(table), controls.checkbutton_unique, 1, 2, 3, 4, GTK_FILL, GTK_SHRINK, 0, 0);

    /* Table properties */
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table),10);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 0);

    /* Initialize states */
    if(!xmlStrcmp(prop_type, (xmlChar*)"file")){
      controls.menu_type=MENUFILE;
      gtk_widget_set_sensitive(controls.hbox_source,TRUE);
      gtk_widget_set_sensitive(controls.label_source,TRUE);
      gtk_widget_set_sensitive(controls.label_style,FALSE);
      gtk_widget_set_sensitive(controls.optionmenu_style,FALSE);
      gtk_widget_set_sensitive(controls.checkbutton_unique,FALSE);
    }else{
      gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_type),1);
      controls.menu_type=SYSTEM;
      gtk_widget_set_sensitive(controls.hbox_source,FALSE);
      gtk_widget_set_sensitive(controls.label_source,FALSE);
      gtk_widget_set_sensitive(controls.label_style,TRUE);
      gtk_widget_set_sensitive(controls.optionmenu_style,TRUE);
      gtk_widget_set_sensitive(controls.checkbutton_unique,TRUE);
    }

    if(!xmlStrcmp(prop_unique, (xmlChar*)"true"))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.checkbutton_unique), TRUE);
    if(!xmlStrcmp(prop_style, (xmlChar*)"multilevel"))
      gtk_option_menu_set_history(GTK_OPTION_MENU(controls.optionmenu_style),1);

    gtk_window_set_default_size(GTK_WINDOW(dialog),300,100);
  }else if(!xmlStrcmp(node->name,(xmlChar*)"app")){
    GtkWidget* name_label;
    GtkWidget* command_label;
    GtkWidget* table;
    GtkWidget *button_browse;
    GtkWidget *hbox_command;
    GtkWidget *icon_label;
    GtkWidget *button_browse2;
    GtkWidget *hbox_icon;

    table = gtk_table_new(4,2,FALSE);

    /* Icon */
    hbox_icon = gtk_hbox_new(FALSE, 0);
    icon_label = gtk_label_new(_("Icon:"));
    icon_entry = gtk_entry_new();
    button_browse2 = gtk_button_new_with_label("...");
    gtk_box_pack_start (GTK_BOX (hbox_icon), icon_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_icon), button_browse2, FALSE, FALSE, 0);
    g_signal_connect ((gpointer) button_browse2, "clicked", G_CALLBACK (browse_icon_cb), icon_entry);

    hbox_command = gtk_hbox_new(FALSE,0);
    name_label = gtk_label_new(_("Name:"));
    command_label = gtk_label_new(_("Command:"));

    name_entry = gtk_entry_new();
    command_entry = gtk_entry_new();

    button_browse = gtk_button_new_with_label("...");

    gtk_box_pack_start (GTK_BOX (hbox_command), command_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_command), button_browse, FALSE, FALSE, 0);
    g_signal_connect ((gpointer) button_browse, "clicked", G_CALLBACK (browse_command_cb), command_entry);

    checkbutton_snotify = gtk_check_button_new_with_mnemonic(_("Use startup _notification"));
    checkbutton_term = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));

    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table),10);

    gtk_table_attach(GTK_TABLE(table), name_label, 0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), name_entry, 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), command_label, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), hbox_command, 1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), icon_label, 0, 1, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), hbox_icon, 1, 2, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), checkbutton_term, 0, 1, 3, 4, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), checkbutton_snotify, 1, 2, 3, 4, GTK_FILL, GTK_SHRINK, 0, 0);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 0);

    if(prop_name)
      gtk_entry_set_text(GTK_ENTRY(name_entry), prop_name);
    if(prop_cmd)
      gtk_entry_set_text(GTK_ENTRY(command_entry), prop_cmd);
    if(prop_icon)
      gtk_entry_set_text(GTK_ENTRY(icon_entry), prop_icon);

    if(prop_snotify && !xmlStrcmp(prop_snotify, (xmlChar*)"true"))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_snotify), TRUE);
    if(prop_term && !xmlStrcmp(prop_term, (xmlChar*)"yes"))
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_term), TRUE);

    gtk_window_set_default_size(GTK_WINDOW(dialog),300,150);
  }else if(!xmlStrcmp(node->name,(xmlChar*)"menu")){
    GtkWidget* name_label;
    GtkWidget* table;
    GtkWidget *icon_label;
    GtkWidget *button_browse;
    GtkWidget* hbox_icon;

    name_label = gtk_label_new(_("Name:"));
    name_entry = gtk_entry_new();

    /* Icon */
    hbox_icon = gtk_hbox_new(FALSE, 0);
    icon_label = gtk_label_new(_("Icon:"));
    icon_entry = gtk_entry_new();
    button_browse = gtk_button_new_with_label("...");
    gtk_box_pack_start (GTK_BOX (hbox_icon), icon_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_icon), button_browse, FALSE, FALSE, 0);
    g_signal_connect ((gpointer) button_browse, "clicked", G_CALLBACK (browse_icon_cb), icon_entry);

    table = gtk_table_new(3,2,FALSE);

    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table),10);

    gtk_table_attach(GTK_TABLE(table), name_label, 0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), name_entry, 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), icon_label, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), hbox_icon, 1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 0);

    if(prop_name)
      gtk_entry_set_text(GTK_ENTRY(name_entry), prop_name);
    if(prop_icon)
      gtk_entry_set_text(GTK_ENTRY(icon_entry), prop_icon);

    gtk_window_set_default_size(GTK_WINDOW(dialog),200,100);
  }else if(!xmlStrcmp(node->name,(xmlChar*)"builtin")){
    GtkWidget* name_label;
    GtkWidget* table;
    GtkWidget *icon_label;
    GtkWidget *button_browse;
    GtkWidget *hbox_icon;

    name_label = gtk_label_new(_("Name:"));
    name_entry = gtk_entry_new();

    /* Icon */
    hbox_icon = gtk_hbox_new(FALSE, 0);
    icon_label = gtk_label_new(_("Icon:"));
    icon_entry = gtk_entry_new();
    button_browse = gtk_button_new_with_label("...");
    gtk_box_pack_start (GTK_BOX (hbox_icon), icon_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_icon), button_browse, FALSE, FALSE, 0);
    g_signal_connect ((gpointer) button_browse, "clicked", G_CALLBACK (browse_icon_cb), icon_entry);
    
    table = gtk_table_new(2,2,FALSE);

    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table),10);

    gtk_table_attach(GTK_TABLE(table), name_label, 0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), name_entry, 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), icon_label, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), hbox_icon, 1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 0);

    if(prop_name)
      gtk_entry_set_text(GTK_ENTRY(name_entry), prop_name);
    if(prop_icon)
      gtk_entry_set_text(GTK_ENTRY(icon_entry), prop_icon);

    gtk_window_set_default_size(GTK_WINDOW(dialog),200,100);
  }else if(!xmlStrcmp(node->name,(xmlChar*)"title")){
    GtkWidget* name_label;
    GtkWidget* table;
    GtkWidget *icon_label;
    GtkWidget *button_browse;
    GtkWidget *hbox_icon;

    table = gtk_table_new(3,2,FALSE);

    /* Icon */
    hbox_icon = gtk_hbox_new(FALSE, 0);
    icon_label = gtk_label_new(_("Icon:"));
    icon_entry = gtk_entry_new();
    button_browse = gtk_button_new_with_label("...");
    gtk_box_pack_start (GTK_BOX (hbox_icon), icon_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_icon), button_browse, FALSE, FALSE, 0);
    g_signal_connect ((gpointer) button_browse, "clicked", G_CALLBACK (browse_icon_cb), icon_entry);

    name_label = gtk_label_new(_("Title:"));
    name_entry = gtk_entry_new();

    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table),10);

    gtk_table_attach(GTK_TABLE(table), name_label, 0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), name_entry, 1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), icon_label, 0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), hbox_icon, 1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 0);

    if(prop_name)
      gtk_entry_set_text(GTK_ENTRY(name_entry), prop_name);
    if(prop_icon)
      gtk_entry_set_text(GTK_ENTRY(icon_entry), prop_icon);

    gtk_window_set_default_size(GTK_WINDOW(dialog),200,100);
  }

  gtk_widget_show_all(dialog);

  /* Commit change if needed */
  if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK){
    if(!xmlStrcmp(node->name,(xmlChar*)"app")){
      GdkPixbuf *icon = NULL;
      gchar *name=NULL;
      gchar *command=NULL;
      GValue val_icon = {0,};

      /* Test if the command exists */
      if(!command_exists(gtk_entry_get_text(GTK_ENTRY(command_entry)))){
	GtkWidget *dialog_warning = gtk_message_dialog_new (GTK_WINDOW(menueditor_app.main_window),
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_WARNING,
							    GTK_BUTTONS_OK,
							    _("The command doesn't exist !"));
	gtk_dialog_run (GTK_DIALOG (dialog_warning));
	gtk_widget_destroy (dialog_warning);
	gtk_widget_destroy (dialog);
	return;
      }

      xmlSetProp(node,"name",gtk_entry_get_text(GTK_ENTRY(name_entry)));
      xmlSetProp(node,"cmd",gtk_entry_get_text(GTK_ENTRY(command_entry)));

      /* unref the icon */
      gtk_tree_model_get_value (GTK_TREE_MODEL(menueditor_app.treestore), &iter, ICON_COLUMN, &val_icon);
      icon = g_value_get_object(&val_icon);

      if(icon){
	g_object_unref(icon);
	icon = NULL;
      }

      if(strlen(gtk_entry_get_text(GTK_ENTRY(icon_entry)))==0){
	xmlAttrPtr icon_prop;
	
	icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	/* Remove the property in the xml tree */
	icon_prop = xmlHasProp(node, "icon");
	xmlRemoveProp(icon_prop);
      }else{
	icon = xfce_themed_icon_load((gchar*) gtk_entry_get_text(GTK_ENTRY(icon_entry)), ICON_SIZE);
	if(!icon)
	  icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	xmlSetProp(node,"icon",gtk_entry_get_text(GTK_ENTRY(icon_entry)));
      }


      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_snotify)))
	xmlSetProp(node, "snotify", "true");
      else{
	xmlAttrPtr snotify_prop;

	/* Remove the property in the xml tree */
	snotify_prop = xmlHasProp(node, "snotify");
	xmlRemoveProp(snotify_prop);
      }
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_term)))
	xmlSetProp(node, "term", "yes");
      else{
	xmlAttrPtr term_prop;

	/* Remove the property in the xml tree */
	term_prop = xmlHasProp(node, "term");
	xmlRemoveProp(term_prop);
      }

      name = g_strdup_printf(NAME_FORMAT,
			     gtk_entry_get_text(GTK_ENTRY(name_entry)));
      command = g_strdup_printf(COMMAND_FORMAT,
				gtk_entry_get_text(GTK_ENTRY(command_entry)));
				      
      gtk_tree_store_set (menueditor_app.treestore, &iter, 
			  ICON_COLUMN, icon,
			  NAME_COLUMN, name,
			  COMMAND_COLUMN, command, -1);

      g_free(name);
      g_free(command);
    }else if(!xmlStrcmp(node->name,(xmlChar*)"menu")){
      gchar *name = NULL;
      GdkPixbuf *icon = NULL;
      GValue val_icon = {0,};

      name = g_strdup_printf(MENU_FORMAT,
			     gtk_entry_get_text(GTK_ENTRY(name_entry)));

      xmlSetProp(node,"name",gtk_entry_get_text(GTK_ENTRY(name_entry)));

      /* unref the icon */
      gtk_tree_model_get_value (GTK_TREE_MODEL(menueditor_app.treestore), &iter, ICON_COLUMN, &val_icon);
      icon = g_value_get_object(&val_icon);

      if(icon){
	g_object_unref(icon);
	icon = NULL;
      }

      if(strlen(gtk_entry_get_text(GTK_ENTRY(icon_entry)))==0){
	xmlAttrPtr icon_prop;

	icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);
	
	/* Remove the property in the xml tree */
	icon_prop = xmlHasProp(node, "icon");
	xmlRemoveProp(icon_prop);
      }else{
	icon = xfce_themed_icon_load((gchar*) gtk_entry_get_text(GTK_ENTRY(icon_entry)), ICON_SIZE);
	if(!icon)
	  icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	xmlSetProp(node,"icon",gtk_entry_get_text(GTK_ENTRY(icon_entry)));
      }
      gtk_tree_store_set (menueditor_app.treestore, &iter,
			  ICON_COLUMN, icon, 
			  NAME_COLUMN, name, -1);
	    
      g_free(name);
    }else if(!xmlStrcmp(node->name,(xmlChar*)"builtin")){
      GdkPixbuf *icon = NULL;
      gchar *name=NULL;
      GValue val_icon = {0,};

      name = g_strdup_printf(BUILTIN_FORMAT,
			     gtk_entry_get_text(GTK_ENTRY(name_entry)));

      xmlSetProp(node,"name",gtk_entry_get_text(GTK_ENTRY(name_entry)));

      /* unref the icon */
      gtk_tree_model_get_value (GTK_TREE_MODEL(menueditor_app.treestore), &iter, ICON_COLUMN, &val_icon);
      icon = g_value_get_object(&val_icon);

      if(icon){
	g_object_unref(icon);
	icon = NULL;
      }

      if(strlen(gtk_entry_get_text(GTK_ENTRY(icon_entry)))==0){
	xmlAttrPtr icon_prop;
	
	icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	/* Remove the property in the xml tree */
	icon_prop = xmlHasProp(node, "icon");
	xmlRemoveProp(icon_prop);
      }else{
	icon = xfce_themed_icon_load((gchar*) gtk_entry_get_text(GTK_ENTRY(icon_entry)), ICON_SIZE);
	if(!icon)
	  icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	xmlSetProp(node,"icon",gtk_entry_get_text(GTK_ENTRY(icon_entry)));
      }

      gtk_tree_store_set (menueditor_app.treestore, &iter,
			  ICON_COLUMN, icon, 
			  NAME_COLUMN, name, -1);
	    

      g_free(name);
    }else if(!xmlStrcmp(node->name,(xmlChar*)"title")){
      GdkPixbuf *icon = NULL;
      gchar *name = NULL;
      GValue val_icon = {0,};

      name = g_strdup_printf(TITLE_FORMAT,
			     gtk_entry_get_text(GTK_ENTRY(name_entry)));

      xmlSetProp(node,"name",gtk_entry_get_text(GTK_ENTRY(name_entry)));
      
      /* unref the icon */
            gtk_tree_model_get_value (GTK_TREE_MODEL(menueditor_app.treestore), &iter, ICON_COLUMN, &val_icon);
      icon = g_value_get_object(&val_icon);

      if(icon){
	g_object_unref(icon);
	icon = NULL;
      }

      if(strlen(gtk_entry_get_text(GTK_ENTRY(icon_entry)))==0){
	xmlAttrPtr icon_prop;

	icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	/* Remove the property in the xml tree */
	icon_prop = xmlHasProp(node, "icon");
	xmlRemoveProp(icon_prop);
      }else{
	icon = xfce_themed_icon_load((gchar*) gtk_entry_get_text(GTK_ENTRY(icon_entry)), ICON_SIZE);
	if(!icon)
	  icon = xfce_inline_icon_at_size(dummy_icon_data, ICON_SIZE, ICON_SIZE);

	xmlSetProp(node,"icon",gtk_entry_get_text(GTK_ENTRY(icon_entry)));
      }

      gtk_tree_store_set (menueditor_app.treestore, &iter,
			  ICON_COLUMN, icon, 
			  NAME_COLUMN, name, -1);


      g_free(name);
    }else if(!xmlStrcmp(node->name,(xmlChar*)"include")){
      gchar *name=NULL;
      gchar *source=NULL;
      xmlAttrPtr unique_prop;
      xmlAttrPtr style_prop;

      switch(controls.menu_type){
      case MENUFILE:
	name = g_strdup_printf(INCLUDE_FORMAT,
			       _("--- include ---"));
	source = g_strdup_printf(INCLUDE_PATH_FORMAT,
				 gtk_entry_get_text(GTK_ENTRY(entry_source)));

	gtk_tree_store_set (menueditor_app.treestore, &iter, 0, NULL, 
			    NAME_COLUMN, name,
			    COMMAND_COLUMN, source,-1);

	xmlSetProp(node,"type", "file");
	xmlSetProp(node,"src",gtk_entry_get_text(GTK_ENTRY(entry_source)));
	
	/* remove unique and style props if needed */
	unique_prop = xmlHasProp(node, "unique");
	xmlRemoveProp(unique_prop);
	style_prop = xmlHasProp(node, "style");
	xmlRemoveProp(style_prop);

	g_free(source);
	g_free(name);
	break;
      case SYSTEM:
	name = g_strdup_printf(INCLUDE_FORMAT,
			       _("--- include ---"));
	source = g_strdup_printf(INCLUDE_PATH_FORMAT,
				 _("system"));

	gtk_tree_store_set (menueditor_app.treestore, &iter, 0, NULL, 
			    NAME_COLUMN, name,
			    COMMAND_COLUMN, source,-1);

	xmlSetProp(node,"type", "system");
	
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(controls.checkbutton_unique))){
	  unique_prop = xmlHasProp(node, "unique");
	  xmlRemoveProp(unique_prop);
	}
	else
	  xmlSetProp(node,"unique","true");

	if(gtk_option_menu_get_history(GTK_OPTION_MENU(controls.optionmenu_style)) == 0){
	  /* remove src prop if needed */
	  style_prop = xmlHasProp(node, "style");
	  xmlRemoveProp(style_prop);
	}else
	  xmlSetProp(node,"style","multilevel");
      }
    
    }
    menueditor_app.menu_modified=TRUE;
    gtk_widget_set_sensitive(menueditor_app.file_menu.save,TRUE);
    gtk_widget_set_sensitive(menueditor_app.main_toolbar.save,TRUE);
  }
  xmlFree(prop_name);
  xmlFree(prop_cmd);
  xmlFree(prop_icon);
  xmlFree(prop_type);
  xmlFree(prop_src);
  xmlFree(prop_snotify);
  xmlFree(prop_term);
  xmlFree(prop_style);
  xmlFree(prop_unique);

  gtk_widget_destroy (dialog);  
}

/* Edition */
void popup_edit_cb(GtkWidget *widget, gpointer data)
{
  edit_selection(gtk_tree_view_get_selection (GTK_TREE_VIEW (menueditor_app.treeview)));
}

void treeview_activate_cb(GtkWidget *widget, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data)
{
  edit_selection(gtk_tree_view_get_selection (GTK_TREE_VIEW (menueditor_app.treeview)));
}

