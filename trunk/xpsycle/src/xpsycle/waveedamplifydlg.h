
#include <ngrs/ndialog.h>

class NEdit;
class NSlider;

namespace psycle
{
	namespace host
	{
		#define AMP_DIALOG_CANCEL -10000

		/// wave amplification dialog window.
		class WaveEdAmplifyDialog : public NDialog
		{
		// Construction
		public:
			WaveEdAmplifyDialog();
		// Dialog Data
			NEdit *m_dbedit;
			NSlider *m_slider;
			
			int db_i;
		protected:
			void onOkClicked(NButtonEvent* ev);
			void onCancelClicked(NButtonEvent* ev);
			void onSliderPosChanged(NSlider*, double pos);
		};

}}
