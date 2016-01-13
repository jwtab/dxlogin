
#ifndef SMS_FIELD_H_
#define SMS_FIELD_H_

#include "dllmain.h"

//The indexes of each of the fields in our credential provider's tiles.
enum SMS_FIELD_ID
{
	//SFI_CODE_TYPE_COMBOBOX,
	//SFI_CODE_BUTTON, 
	SFI_CODE_TYPE_CHECK_BOX,
	
	SFI_MAX_FIELDS,  // Note: if new fields are added, keep NUM_FIELDS last.  This is used as a count of the number of fields
};

// The field state value indicates whether the field is displayed
// in the selected tile, the deselected tile, or both.
// The Field interactive state indicates when 
static FIELD_STATE_PAIR s_rgFieldStatePairs[] = 
{
	//{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE},       //SFI_CODE_BUTTON   
	//{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE},       //SFI_CODE_TYPE_COMBOBOX
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE},       //SFI_CODE_TYPE_CHECK_BOX
};

// Field descriptors for unlock and logon.
// The first field is the index of the field.
// The second is the type of the field.
// The third is the name of the field, NOT the value which will appear in the field.
static CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgCredProvFieldDescriptors[] =
{
	//{ SFI_CODE_TYPE_COMBOBOX, CPFT_COMBOBOX, L" "},
	//{ SFI_CODE_BUTTON, CPFT_COMMAND_LINK, L"Get login code"},	
	{ SFI_CODE_TYPE_CHECK_BOX,CPFT_CHECKBOX,L" ÷∂Ø ‰»Î√‹¬Î"},
};

static const PWSTR s_gLoginComboBoxStrings[] =
{
	L"SMS",
	L"EMail",
	L"XLog",
};

#endif