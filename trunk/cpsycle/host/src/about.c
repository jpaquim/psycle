// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "about.h"
// host
#include "resources/resource.h"
#include "styles.h"
// platform
#include "../../detail/portable.h"

void contrib_init(Contrib* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(contrib_base(self), parent, NULL);
	psy_ui_component_setdefaultalign(contrib_base(self), psy_ui_ALIGN_TOP,
		psy_ui_defaults_vmargin(psy_ui_defaults()));
	psy_ui_edit_multiline_init(&self->contrib, contrib_base(self));
	psy_ui_edit_preventedit(&self->contrib);
	psy_ui_edit_setlinenumber(&self->contrib, 10);
	psy_ui_edit_settext(&self->contrib,						
		"Josep Mª Antolín. [JAZ]/JosepMa\tDeveloper since release 1.5" "\r\n"
		"Johan Boulé [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
		"Stefan Nattkemper\t\t\tDeveloper of the LUA host in release 1.12" "\r\n"
		"James Redfern [alk]\t\tDeveloper and plugin coder" "\r\n"
		"Magnus Jonsson [zealmange]\t\tDeveloper during 1.7.x and 1.9alpha" "\r\n"
		"Jeremy Evers [pooplog]\t\tDeveloper in releases 1.7.x" "\r\n"
		"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
		"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
		"Mark McCormack\t\t\tMIDI (in) Support in release 1.6" "\r\n"
		"Mats Höjlund\t\t\tMain Developer until release 1.5" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
		"Juan Antonio Arguelles [Arguru] (RIP)\tOriginal Developer and maintainer until 1.0" "\r\n"
		"Satoshi Fujiwara\t\t\tBase code for the Sampulse machine\r\n"
		"Hermann Seib\t\t\tBase code for the new VST Host in 1.8.5\r\n"
		"Martin Etnestad Johansen [lobywang]\tCoding Help" "\r\n"
		"Patrick Haworth [TranceMyriad]\tAuthor of the Help File" "\r\n"
		"Budislav Stepanov\t\t\tNew artwork for version 1.11\r\n"
		"Angelus\t\t\t\tNew Skin for 1.10, example songs, beta tester" "\r\n"
		"ikkkle\t\t\t\tNew toolbar graphics for 1.10.0" "\r\n"
		"Hamarr Heylen\t\t\tInitial Graphics" "\r\n"
		"David Buist\t\t\tAdditional Graphics" "\r\n"
		"frown\t\t\t\tAdditional Graphics" "\r\n"
		"/\\/\\ark\t\t\t\tAdditional Graphics" "\r\n"
		"Michael Haralabos\t\t\tInstaller and Debugging help" "\r\n\r\n"
		"This release of Psycle also contains VST plugins from:" "\r\n"
		"Digital Fish Phones\t( http://www.digitalfishphones.com/ )" "\r\n"
		"DiscoDSP\t\t( http://www.discodsp.com/ )" "\r\n"
		"SimulAnalog\t( http://www.simulanalog.org/ )" "\r\n"
		"Jeroen Breebaart\t( http://www.jeroenbreebaart.com/ )" "\r\n"
		"George Yohng\t( http://www.yohng.com/ )" "\r\n"
		"Christian Budde\t( http://www.savioursofsoul.de/Christian/ )" "\r\n"
		"DDMF\t\t( http://www.ddmf.eu/ )" "\r\n"
		"Loser\t\t( http://loser.asseca.com/ )" "\r\n"
		"E-phonic\t\t( http://www.e-phonic.com/ )" "\r\n"
		"Argu\t\t( http://www.aodix.com/ )" "\r\n"
		"Oatmeal by Fuzzpilz\t( http://bicycle-for-slugs.org/ )"
	);	
	psy_ui_edit_init(&self->psycledelics, contrib_base(self));
	psy_ui_edit_preventedit(&self->psycledelics);
	psy_ui_edit_settext(&self->psycledelics, "http://psycle.pastnotecut.org");	
	psy_ui_edit_init(&self->sourceforge, contrib_base(self));
	psy_ui_edit_preventedit(&self->sourceforge);
	psy_ui_edit_settext(&self->sourceforge, "http://psycle.sourceforge.net");
	psy_ui_label_init(&self->steincopyright, contrib_base(self), NULL);	
	psy_ui_label_preventtranslation(&self->steincopyright);
	psy_ui_label_settext(&self->steincopyright,
		"VST Virtual Studio Technology v2.4 (c)1998-2006 Steinberg");	
}

void version_init(Version* self, psy_ui_Component* parent)
{
	psy_ui_component_init(version_base(self), parent, NULL);
	psy_ui_label_init(&self->versioninfo, version_base(self), NULL);
	psy_ui_label_preventtranslation(&self->versioninfo);
	psy_ui_label_settextalignment(&self->versioninfo, psy_ui_ALIGNMENT_CENTER_HORIZONTAL);
	psy_ui_label_settext(&self->versioninfo, PSYCLE__BUILD__IDENTIFIER("\r\n"));
	psy_ui_component_resize(psy_ui_label_base(&self->versioninfo),
		psy_ui_size_makepx(500, 300));
}

// Version
static void licence_set_en(Licence*);
static void licence_set_es(Licence*);
static void licence_setlanguage(Licence*);
static void licence_onlanguagechanged(Licence*, psy_Translator* sender);

void licence_init(Licence* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_signal_connect(&psy_ui_app()->translator.signal_languagechanged,
		self, licence_onlanguagechanged);
	psy_ui_editor_init(&self->licenceinfo, &self->component);	
	licence_setlanguage(self);
	psy_ui_component_resize(&self->licenceinfo.component,
		psy_ui_size_makepx(500.0, 300.0));
	psy_ui_component_setbackgroundcolour(&self->licenceinfo.component,
		psy_ui_style(psy_ui_STYLE_COMMON)->backgroundcolour);
	psy_ui_editor_preventedit(&self->licenceinfo);
	psy_ui_editor_enablewrap(&self->licenceinfo);
}

void licence_set_en(Licence* self)
{
	psy_ui_editor_settext(&self->licenceinfo,
		"Psycle is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version."
		"copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net\r\n\r\n"
		"GNU GENERAL PUBLIC LICENSE\r\n"
		"Version 2, June 1991\r\n"
		"Copyright(C) 1989, 1991 Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301, USA Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.\r\n"
		"Preamble\r\n"
		"The licenses for most software are designed to take away your freedom to shareand change it.By contrast, the GNU General Public License is intended to guarantee your freedom to shareand change free software--to make sure the software is free for all its users.This General Public License applies to most of the Free Software Foundation's software and to any other program whose authors commit to using it. (Some other Free Software Foundation software is covered by the GNU Library General Public License instead.) You can apply it to your programs, too. "
		"When we speak of free software, we are referring to freedom, not price.Our General Public Licenses are designed to make sure that you have the freedom to distribute copies of free software(and charge for this service if you wish), that you receive source code or can get it if you want it, that you can change the software or use pieces of it in new free programs;and that you know you can do these things."
		"To protect your rights, we need to make restrictions that forbid anyone to deny you these rights or to ask you to surrender the rights.These restrictions translate to certain responsibilities for you if you distribute copies of the software, or if you modify it."
		"For example, if you distribute copies of such a program, whether gratis or for a fee, you must give the recipients all the rights that you have.You must make sure that they, too, receive or can get the source code.And you must show them these terms so they know their rights."
		"We protect your rights with two steps : (1) copyright the software, and (2) offer you this license which gives you legal permission to copy, distributeand /or modify the software."
		"Also, for each author's protection and ours, we want to make certain that everyone understands that there is no warranty for this free software. If the software is modified by someone else and passed on, we want its recipients to know that what they have is not the original, so that any problems introduced by others will not reflect on the original authors' reputations."
		"Finally, any free program is threatened constantly by software patents.We wish to avoid the danger that redistributors of a free program will individually obtain patent licenses, in effect making the program proprietary.To prevent this, we have made it clear that any patent must be licensed for everyone's free use or not licensed at all. "
		"The precise terms and conditions for copying, distributionand modification follow.\r\n"
		"GNU GENERAL PUBLIC LICENSE\r\n"
		"TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\r\n"
		"0. This License applies to any program or other work which contains a notice placed by the copyright holder saying it may be distributed under the terms of this General Public License.The \"Program\", below, refers to any such program or work, and a \"work based on the Program\" means either the Program or any derivative work under copyright law : that is to say, a work containing the Program or a portion of it, either verbatim or with modifications and /or translated into another language. (Hereinafter, translation is included without limitation in the term \"modification\".) Each licensee is addressed as \"you\"."
		"Activities other than copying, distributionand modification are not covered by this License; they are outside its scope.The act of running the Program is not restricted, and the output from the Program is covered only if its contents constitute a work based on the Program(independent of having been made by running the Program).Whether that is true depends on what the Program does."
		"1. You may copy and distribute verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice and disclaimer of warranty; keep intact all the notices that refer to this License and to the absence of any warranty; and give any other recipients of the Program a copy of this License along with the Program. "
		"You may charge a fee for the physical act of transferring a copy, and you may at your option offer warranty protection in exchange for a fee."
		"2. You may modify your copy or copies of the Program or any portion of it, thus forming a work based on the Program, and copyand distribute such modifications or work under the terms of Section 1 above, provided that you also meet all of these conditions :"
		"a) You must cause the modified files to carry prominent notices stating that you changed the filesand the date of any change."
		"b) You must cause any work that you distribute or publish, that in whole or in part contains or is derived from the Program or any part thereof, to be licensed as a whole at no charge to all third parties under the terms of this License."
		"c) If the modified program normally reads commands interactively when run, you must cause it, when started running for such interactive use in the most ordinary way, to print or display an announcement including an appropriate copyright notice and a notice that there is no warranty(or else, saying that you provide a warranty) and that users may redistribute the program under these conditions, and telling the user how to view a copy of this License. (Exception: if the Program itself is interactive but does not normally print such an announcement, your work based on the Program is not required to print an announcement.)"
		"These requirements apply to the modified work as a whole.If identifiable sections of that work are not derived from the Program, and can be reasonably considered independentand separate works in themselves, then this License, and its terms, do not apply to those sections when you distribute them as separate works.But when you distribute the same sections as part of a whole which is a work based on the Program, the distribution of the whole must be on the terms of this License, whose permissions for other licensees extend to the entire whole, and thus to eachand every part regardless of who wrote it."
		"Thus, it is not the intent of this section to claim rights or contest your rights to work written entirely by you; rather, the intent is to exercise the right to control the distribution of derivative or collective works based on the Program."
		"In addition, mere aggregation of another work not based on the Program with the Program(or with a work based on the Program) on a volume of a storage or distribution medium does not bring the other work under the scope of this License."
		"3. You may copy and distribute the Program(or a work based on it, under Section 2) in object code or executable form under the terms of Sections 1 and 2 above provided that you also do one of the following :"
		"a) Accompany it with the complete corresponding machine - readable source code, which must be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,"
		"b) Accompany it with a written offer, valid for at least three years, to give any third party, for a charge no more than your cost of physically performing source distribution, a complete machine - readable copy of the corresponding source code, to be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,"
		"c) Accompany it with the information you received as to the offer to distribute corresponding source code. (This alternative is allowed only for noncommercial distributionand only if you received the program in object code or executable form with such an offer, in accord with Subsection b above.)"
		"The source code for a work means the preferred form of the work for making modifications to it.For an executable work, complete source code means all the source code for all modules it contains, plus any associated interface definition files, plus the scripts used to control compilationand installation of the executable.However, as a special exception, the source code distributed need not include anything that is normally distributed(in either source or binary form) with the major components(compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable."
		"If distribution of executable or object code is made by offering access to copy from a designated place, then offering equivalent access to copy the source code from the same place counts as distribution of the source code, even though third parties are not compelled to copy the source along with the object code."
		"4. You may not copy, modify, sublicense, or distribute the Program except as expressly provided under this License.Any attempt otherwise to copy, modify, sublicense or distribute the Program is void, and will automatically terminate your rights under this License.However, parties who have received copies, or rights, from you under this License will not have their licenses terminated so long as such parties remain in full compliance."
		"5. You are not required to accept this License, since you have not signed it.However, nothing else grants you permission to modify or distribute the Program or its derivative works.These actions are prohibited by law if you do not accept this License.Therefore, by modifying or distributing the Program(or any work based on the Program), you indicate your acceptance of this License to do so, and all its termsand conditions for copying, distributing or modifying the Program or works based on it."
		"6. Each time you redistribute the Program(or any work based on the Program), the recipient automatically receives a license from the original licensor to copy, distribute or modify the Program subject to these terms and conditions.You may not impose any further restrictions on the recipients' exercise of the rights granted herein. You are not responsible for enforcing compliance by third parties to this License. "
		"7. If, as a consequence of a court judgment or allegation of patent infringement or for any other reason(not limited to patent issues), conditions are imposed on you(whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License.If you cannot distribute so as to satisfy simultaneously your obligations under this Licenseand any other pertinent obligations, then as a consequence you may not distribute the Program at all.For example, if a patent license would not permit royalty - free redistribution of the Program by all those who receive copies directly or indirectly through you, then the only way you could satisfy both itand this License would be to refrain entirely from distribution of the Program."
		"If any portion of this section is held invalid or unenforceable under any particular circumstance, the balance of the section is intended to applyand the section as a whole is intended to apply in other circumstances."
		"It is not the purpose of this section to induce you to infringe any patents or other property right claims or to contest validity of any such claims; this section has the sole purpose of protecting the integrity of the free software distribution system, which is implemented by public license practices.Many people have made generous contributions to the wide range of software distributed through that system in reliance on consistent application of that system; it is up to the author / donor to decide if he or she is willing to distribute software through any other system and a licensee cannot impose that choice."
		"This section is intended to make thoroughly clear what is believed to be a consequence of the rest of this License."
		"8. If the distribution and /or use of the Program is restricted in certain countries either by patents or by copyrighted interfaces, the original copyright holder who places the Program under this License may add an explicit geographical distribution limitation excluding those countries, so that distribution is permitted only in or among countries not thus excluded.In such case, this License incorporates the limitation as if written in the body of this License."
		"9. The Free Software Foundation may publish revised and /or new versions of the General Public License from time to time.Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns."
		"Each version is given a distinguishing version number.If the Program specifies a version number of this License which applies to it and \"any later version\", you have the option of following the termsand conditions either of that version or of any later version published by the Free Software Foundation.If the Program does not specify a version number of this License, you may choose any version ever published by the Free Software Foundation."
		"10. If you wish to incorporate parts of the Program into other free programs whose distribution conditions are different, write to the author to ask for permission.For software which is copyrighted by the Free Software Foundation, write to the Free Software Foundation; we sometimes make exceptions for this.Our decision will be guided by the two goals of preserving the free status of all derivatives of our free softwareand of promoting the sharingand reuse of software generally."
		"NO WARRANTY"
		"11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND / OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION."
		"12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND / OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM(INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\r\n"
	);
}

void licence_set_es(Licence* self)
{	
	psy_ui_editor_settext(&self->licenceinfo,
		"Psycle es software libre; puede redistribuirla o modificarla bajo los términos de la Licencia Pública General GNU publicada por la «Free Software Foundation» ; bien de la versión 2 de dicha Licencia o bien (según su elección) de cualquier versión posterior."
		"copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net\r\n\r\n"
		"GNU GENERAL PUBLIC LICENSE\r\n"
		"Version 2, June 1991\r\n"
		"Copyright(C) 1989, 1991 Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301, USA Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.\r\n"
		"Preamble\r\n"
		"The licenses for most software are designed to take away your freedom to shareand change it.By contrast, the GNU General Public License is intended to guarantee your freedom to shareand change free software--to make sure the software is free for all its users.This General Public License applies to most of the Free Software Foundation's software and to any other program whose authors commit to using it. (Some other Free Software Foundation software is covered by the GNU Library General Public License instead.) You can apply it to your programs, too. "
		"When we speak of free software, we are referring to freedom, not price.Our General Public Licenses are designed to make sure that you have the freedom to distribute copies of free software(and charge for this service if you wish), that you receive source code or can get it if you want it, that you can change the software or use pieces of it in new free programs;and that you know you can do these things."
		"To protect your rights, we need to make restrictions that forbid anyone to deny you these rights or to ask you to surrender the rights.These restrictions translate to certain responsibilities for you if you distribute copies of the software, or if you modify it."
		"For example, if you distribute copies of such a program, whether gratis or for a fee, you must give the recipients all the rights that you have.You must make sure that they, too, receive or can get the source code.And you must show them these terms so they know their rights."
		"We protect your rights with two steps : (1) copyright the software, and (2) offer you this license which gives you legal permission to copy, distributeand /or modify the software."
		"Also, for each author's protection and ours, we want to make certain that everyone understands that there is no warranty for this free software. If the software is modified by someone else and passed on, we want its recipients to know that what they have is not the original, so that any problems introduced by others will not reflect on the original authors' reputations."
		"Finally, any free program is threatened constantly by software patents.We wish to avoid the danger that redistributors of a free program will individually obtain patent licenses, in effect making the program proprietary.To prevent this, we have made it clear that any patent must be licensed for everyone's free use or not licensed at all. "
		"The precise terms and conditions for copying, distributionand modification follow.\r\n"
		"GNU GENERAL PUBLIC LICENSE\r\n"
		"TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\r\n"
		"0. This License applies to any program or other work which contains a notice placed by the copyright holder saying it may be distributed under the terms of this General Public License.The \"Program\", below, refers to any such program or work, and a \"work based on the Program\" means either the Program or any derivative work under copyright law : that is to say, a work containing the Program or a portion of it, either verbatim or with modifications and /or translated into another language. (Hereinafter, translation is included without limitation in the term \"modification\".) Each licensee is addressed as \"you\"."
		"Activities other than copying, distributionand modification are not covered by this License; they are outside its scope.The act of running the Program is not restricted, and the output from the Program is covered only if its contents constitute a work based on the Program(independent of having been made by running the Program).Whether that is true depends on what the Program does."
		"1. You may copy and distribute verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice and disclaimer of warranty; keep intact all the notices that refer to this License and to the absence of any warranty; and give any other recipients of the Program a copy of this License along with the Program. "
		"You may charge a fee for the physical act of transferring a copy, and you may at your option offer warranty protection in exchange for a fee."
		"2. You may modify your copy or copies of the Program or any portion of it, thus forming a work based on the Program, and copyand distribute such modifications or work under the terms of Section 1 above, provided that you also meet all of these conditions :"
		"a) You must cause the modified files to carry prominent notices stating that you changed the filesand the date of any change."
		"b) You must cause any work that you distribute or publish, that in whole or in part contains or is derived from the Program or any part thereof, to be licensed as a whole at no charge to all third parties under the terms of this License."
		"c) If the modified program normally reads commands interactively when run, you must cause it, when started running for such interactive use in the most ordinary way, to print or display an announcement including an appropriate copyright notice and a notice that there is no warranty(or else, saying that you provide a warranty) and that users may redistribute the program under these conditions, and telling the user how to view a copy of this License. (Exception: if the Program itself is interactive but does not normally print such an announcement, your work based on the Program is not required to print an announcement.)"
		"These requirements apply to the modified work as a whole.If identifiable sections of that work are not derived from the Program, and can be reasonably considered independentand separate works in themselves, then this License, and its terms, do not apply to those sections when you distribute them as separate works.But when you distribute the same sections as part of a whole which is a work based on the Program, the distribution of the whole must be on the terms of this License, whose permissions for other licensees extend to the entire whole, and thus to eachand every part regardless of who wrote it."
		"Thus, it is not the intent of this section to claim rights or contest your rights to work written entirely by you; rather, the intent is to exercise the right to control the distribution of derivative or collective works based on the Program."
		"In addition, mere aggregation of another work not based on the Program with the Program(or with a work based on the Program) on a volume of a storage or distribution medium does not bring the other work under the scope of this License."
		"3. You may copy and distribute the Program(or a work based on it, under Section 2) in object code or executable form under the terms of Sections 1 and 2 above provided that you also do one of the following :"
		"a) Accompany it with the complete corresponding machine - readable source code, which must be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,"
		"b) Accompany it with a written offer, valid for at least three years, to give any third party, for a charge no more than your cost of physically performing source distribution, a complete machine - readable copy of the corresponding source code, to be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,"
		"c) Accompany it with the information you received as to the offer to distribute corresponding source code. (This alternative is allowed only for noncommercial distributionand only if you received the program in object code or executable form with such an offer, in accord with Subsection b above.)"
		"The source code for a work means the preferred form of the work for making modifications to it.For an executable work, complete source code means all the source code for all modules it contains, plus any associated interface definition files, plus the scripts used to control compilationand installation of the executable.However, as a special exception, the source code distributed need not include anything that is normally distributed(in either source or binary form) with the major components(compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable."
		"If distribution of executable or object code is made by offering access to copy from a designated place, then offering equivalent access to copy the source code from the same place counts as distribution of the source code, even though third parties are not compelled to copy the source along with the object code."
		"4. You may not copy, modify, sublicense, or distribute the Program except as expressly provided under this License.Any attempt otherwise to copy, modify, sublicense or distribute the Program is void, and will automatically terminate your rights under this License.However, parties who have received copies, or rights, from you under this License will not have their licenses terminated so long as such parties remain in full compliance."
		"5. You are not required to accept this License, since you have not signed it.However, nothing else grants you permission to modify or distribute the Program or its derivative works.These actions are prohibited by law if you do not accept this License.Therefore, by modifying or distributing the Program(or any work based on the Program), you indicate your acceptance of this License to do so, and all its termsand conditions for copying, distributing or modifying the Program or works based on it."
		"6. Each time you redistribute the Program(or any work based on the Program), the recipient automatically receives a license from the original licensor to copy, distribute or modify the Program subject to these terms and conditions.You may not impose any further restrictions on the recipients' exercise of the rights granted herein. You are not responsible for enforcing compliance by third parties to this License. "
		"7. If, as a consequence of a court judgment or allegation of patent infringement or for any other reason(not limited to patent issues), conditions are imposed on you(whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License.If you cannot distribute so as to satisfy simultaneously your obligations under this Licenseand any other pertinent obligations, then as a consequence you may not distribute the Program at all.For example, if a patent license would not permit royalty - free redistribution of the Program by all those who receive copies directly or indirectly through you, then the only way you could satisfy both itand this License would be to refrain entirely from distribution of the Program."
		"If any portion of this section is held invalid or unenforceable under any particular circumstance, the balance of the section is intended to applyand the section as a whole is intended to apply in other circumstances."
		"It is not the purpose of this section to induce you to infringe any patents or other property right claims or to contest validity of any such claims; this section has the sole purpose of protecting the integrity of the free software distribution system, which is implemented by public license practices.Many people have made generous contributions to the wide range of software distributed through that system in reliance on consistent application of that system; it is up to the author / donor to decide if he or she is willing to distribute software through any other system and a licensee cannot impose that choice."
		"This section is intended to make thoroughly clear what is believed to be a consequence of the rest of this License."
		"8. If the distribution and /or use of the Program is restricted in certain countries either by patents or by copyrighted interfaces, the original copyright holder who places the Program under this License may add an explicit geographical distribution limitation excluding those countries, so that distribution is permitted only in or among countries not thus excluded.In such case, this License incorporates the limitation as if written in the body of this License."
		"9. The Free Software Foundation may publish revised and /or new versions of the General Public License from time to time.Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns."
		"Each version is given a distinguishing version number.If the Program specifies a version number of this License which applies to it and \"any later version\", you have the option of following the termsand conditions either of that version or of any later version published by the Free Software Foundation.If the Program does not specify a version number of this License, you may choose any version ever published by the Free Software Foundation."
		"10. If you wish to incorporate parts of the Program into other free programs whose distribution conditions are different, write to the author to ask for permission.For software which is copyrighted by the Free Software Foundation, write to the Free Software Foundation; we sometimes make exceptions for this.Our decision will be guided by the two goals of preserving the free status of all derivatives of our free softwareand of promoting the sharingand reuse of software generally."
		"NO WARRANTY"
		"11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND / OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION."
		"12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND / OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM(INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\r\n"
	);
}

void licence_onlanguagechanged(Licence* self, psy_Translator* sender)
{
	licence_setlanguage(self);
}

void licence_setlanguage(Licence* self)
{
	if (psy_translator_langid(&psy_ui_app()->translator) &&
			(strcmp(psy_translator_langid(&psy_ui_app()->translator), "es") == 0)) {
		licence_set_es(self);
	} else {
		licence_set_en(self);
	}
}

// About
// prototypes
static void about_initbuttons(About*);
static void about_oncontributors(About*, psy_ui_Component* sender);
static void about_onversion(About*, psy_ui_Component* sender);
static void about_onlicence(About*, psy_ui_Component* sender);
static void about_onmousedoubleclick(About*, psy_ui_MouseEvent*);
static void about_onalign(About*);
static void about_onfocus(About*, psy_ui_Component* sender);
// vtable
static psy_ui_ComponentVtable about_vtable;
static int about_vtable_initialized = 0;

static void about_vtable_init(About* self)
{
	if (!about_vtable_initialized) {
		about_vtable = *(self->component.vtable);
		about_vtable.onalign = (psy_ui_fp_component_onalign)about_onalign;
		about_vtable.onmousedoubleclick = (psy_ui_fp_component_onmouseevent)
			about_onmousedoubleclick;		
		about_vtable_initialized = 1;
	}
}
// implementation
void about_init(About* self, psy_ui_Component* parent, Workspace* workspace)
{				
	psy_ui_component_init(&self->component, parent, NULL);
	about_vtable_init(self);
	self->component.vtable = &about_vtable;
	psy_ui_component_setstyletypes(&self->component,
		STYLE_ABOUT, psy_INDEX_INVALID, psy_INDEX_INVALID,
		psy_INDEX_INVALID);
	self->workspace = workspace;	
	about_initbuttons(self);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_image_init(&self->image, psy_ui_notebook_base(&self->notebook));	
	self->image.component.preventdefault = 0;
	psy_ui_bitmap_loadresource(&self->image.bitmap, IDB_ABOUT);	
	// psy_ui_bitmap_load(&self->image.bitmap,
	//	"/home/user/cpsycle/host/src/resources/splash_screen.png");
	contrib_init(&self->contrib, psy_ui_notebook_base(&self->notebook));
	version_init(&self->version, psy_ui_notebook_base(&self->notebook));
	licence_init(&self->licence, psy_ui_notebook_base(&self->notebook));
	psy_ui_notebook_select(&self->notebook, 0);
	psy_signal_connect(&self->component.signal_focus, self,
		about_onfocus);
}

void about_initbuttons(About* self)
{
	// contrib
	psy_ui_button_init_connect(&self->contribbutton, &self->component, NULL,
		self, about_oncontributors);
	psy_ui_button_settext(&self->contribbutton,
		"help.contributors-credits");
	// version
	psy_ui_button_init_connect(&self->versionbutton, &self->component, NULL,
		self, about_onversion);
	psy_ui_button_preventtranslation(&self->versionbutton);
	psy_ui_button_settext(&self->versionbutton, PSYCLE__VERSION);
	// licence
	psy_ui_button_init_connect(&self->licencebutton, &self->component, NULL,
		self, about_onlicence);	
	psy_ui_button_settext(&self->licencebutton, "help.licence");
	// ok
	psy_ui_button_init(&self->okbutton, &self->component, NULL);
	psy_ui_button_settext(&self->okbutton, "help.ok");
}

void about_onalign(About* self)
{
	psy_ui_Size size;
	psy_ui_RealSize bitmapsize;
	const psy_ui_TextMetric* tm;
	double centerx;
	double centery;
	psy_ui_Size contribbuttonsize;
	psy_ui_Size versionbuttonsize;
	psy_ui_Size licencebuttonsize;
	psy_ui_Size okbuttonsize;
	double charmargin = 25;
	double width = 0;
	double margin;
	double cpx;
	
	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	bitmapsize = psy_ui_bitmap_size(&self->image.bitmap);
	bitmapsize.width = 
		psy_max(tm->tmAveCharWidth * 40, bitmapsize.width);
	bitmapsize.height = psy_max(tm->tmHeight * 20,
			bitmapsize.height + tm->tmHeight * 4);
	centerx = (psy_ui_value_px(&size.width, tm) - bitmapsize.width) / 2;
	centery = (psy_ui_value_px(&size.height, tm) - bitmapsize.height) / 2;
	contribbuttonsize = psy_ui_component_preferredsize(&self->contribbutton.component, &size);
	versionbuttonsize = psy_ui_component_preferredsize(&self->versionbutton.component, &size);
	licencebuttonsize = psy_ui_component_preferredsize(&self->licencebutton.component, &size);
	okbuttonsize = psy_ui_component_preferredsize(&self->okbutton.component, &size);
	if (centery + bitmapsize.height +
		psy_ui_value_px(&okbuttonsize.height, tm) > psy_ui_value_px(&size.height, tm)) {
		bitmapsize.height = psy_ui_value_px(&size.height, tm) -
			psy_ui_value_px(&okbuttonsize.height, tm) * 2;
		centery = (psy_ui_value_px(&size.height, tm) - bitmapsize.height) / 2;
	}
	psy_ui_component_setposition(psy_ui_notebook_base(&self->notebook),
		psy_ui_rectangle_make(
			psy_ui_point_makepx(centerx ,centery),
			psy_ui_size_makepx(bitmapsize.width, bitmapsize.height)));
	do {
		margin = tm->tmAveCharWidth * charmargin;
		width = psy_ui_value_px(&contribbuttonsize.width, tm) +
			psy_ui_value_px(&versionbuttonsize.width, tm) +
			psy_ui_value_px(&licencebuttonsize.width, tm) +
			psy_ui_value_px(&okbuttonsize.width, tm) + margin * 3;
		--charmargin;
	} while (width > psy_ui_value_px(&size.width, tm) && charmargin > 0);
	cpx = (psy_ui_value_px(&size.width, tm) - width) / 2;
	psy_ui_component_setposition(&self->contribbutton.component,
		psy_ui_rectangle_make(
		psy_ui_point_makepx(cpx, centery + bitmapsize.height),
		contribbuttonsize));
	psy_ui_component_setposition(&self->versionbutton.component,
		psy_ui_rectangle_make(
		psy_ui_point_makepx(
			cpx + psy_ui_value_px(&contribbuttonsize.width, tm) + margin,
			centery + bitmapsize.height),
		versionbuttonsize));
	psy_ui_component_setposition(&self->licencebutton.component,
		psy_ui_rectangle_make(
		psy_ui_point_makepx(
			cpx + psy_ui_value_px(&contribbuttonsize.width, tm)
				+ psy_ui_value_px(&versionbuttonsize.width, tm) +
				margin * 2,
			centery + bitmapsize.height),
		licencebuttonsize));
	psy_ui_component_setposition(&self->okbutton.component,
		psy_ui_rectangle_make(
		psy_ui_point_makepx(			
			cpx + psy_ui_value_px(&contribbuttonsize.width, tm) +
				psy_ui_value_px(&versionbuttonsize.width, tm) +
				psy_ui_value_px(&licencebuttonsize.width, tm) + 
				margin * 3,
			centery + bitmapsize.height),
		okbuttonsize));
}

void about_onfocus(About* self, psy_ui_Component* sender)
{
	psy_ui_component_setfocus(&self->okbutton.component);
}

void about_oncontributors(About* self, psy_ui_Component* sender) 
{	
	psy_ui_notebook_select(&self->notebook, 
		psy_ui_notebook_pageindex(&self->notebook) != 1 ? 1 : 0);
	psy_ui_component_invalidate(&self->component);
}

void about_onversion(About* self, psy_ui_Component* sender) 
{	
	psy_ui_notebook_select(&self->notebook, 
		psy_ui_notebook_pageindex(&self->notebook) != 2 ? 2 : 0);
	psy_ui_component_invalidate(&self->component);
}

void about_onlicence(About* self, psy_ui_Component* sender)
{
	psy_ui_notebook_select(&self->notebook,
		psy_ui_notebook_pageindex(&self->notebook) != 3 ? 3 : 0);
	psy_ui_component_invalidate(&self->component);
}

void about_onmousedoubleclick(About* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->okbutton.signal_clicked, self, 0);
}
