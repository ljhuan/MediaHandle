#ifndef MEDIA_ALGORITHM_H_
#define MEDIA_ALGORITHM_H_

#include <string.h>
#include "media_process.h"
#include "media_element.h"
#include "media_type.h"
#include "algorithm/Lpr_Interface.h"

using namespace std;
class MP_H3C_LPR : public BaseMediaProcessPipe {
public:
    MP_H3C_LPR() {}
    virtual const size_t getInputCount() const {
        return 1;
    }

    virtual const size_t getOutputCount() const {
        return 1;
    }

    virtual void input(const size_t &index, const std::shared_ptr<BaseMediaElement> &mediaElement) {
        auto yuv = static_pointer_cast<Yuv420>(mediaElement->getMediaBuffer("yuv"));
        H3C_REC_RESULT_S rslt;
        memset(&rslt, 0, sizeof(H3C_REC_RESULT_S));
        try {
            lprWork(yuv, &rslt);
            mediaElement->setMetadata<uint32_t>("step", 3);
            mediaElement->setMetadata<string>("plate", rslt.acPlateCode);
            outputHandlers_[index](mediaElement);
        } catch (const std::exception& ex) {
            std::cout << "lpr work exception:" << ex.what() << std::endl;
        }
    }

private:
    LPR_INTERFACE_S m_stLprInterface;
    LPR_ROI_INDEX_S m_stRoiSet;         // 裁剪区域
    void *m_hLprHandle = nullptr;

    void lprInit() {
        m_stLprInterface.iDefaultC = 3;  // zhejiang province
        m_stLprInterface.iDefaultL = 0;  // hangzhou city
        m_stLprInterface.usMinWidth = 100;
        m_stLprInterface.iLprPrintEnable = 0;

        /* 线程总数 */
        m_stLprInterface.iThreadTotal = 1;

        /* 系统线程号下发 */
        m_stLprInterface.iThreadID = 0;

        /* DSP车牌识别库初始化 */
        m_stLprInterface.eImgType = IMG_YUV420;
        m_stLprInterface.iBitCnt = 8;

        /* 默认省份默认城市从配置文件读取 */
        m_stLprInterface.usMaxWidth = 160;
        m_stLprInterface.fGammaAdj = 1.0;

        // 重新创建资源
        m_stLprInterface.iImgHgt = 2048;
        m_stLprInterface.iImgWid = 2592;
        m_stLprInterface.iStride = 2592;
        m_stLprInterface.iROICnt = 1;
        m_stLprInterface.pstROISet = &m_stRoiSet;
        // 区域裁减
        m_stLprInterface.pstROISet->iXTopLeft = 0;
        m_stLprInterface.pstROISet->iYTopLeft = 0;
        m_stLprInterface.pstROISet->iXTopRight = 0;
        m_stLprInterface.pstROISet->iYTopRight = 0;
        m_stLprInterface.pstROISet->iXBottmLeft = 0;
        m_stLprInterface.pstROISet->iYBottmLeft = 0;
        m_stLprInterface.pstROISet->iXBottmRight = 0;
        m_stLprInterface.pstROISet->iYBottmRight = 0;
        m_stLprInterface.uiEnableFlag = LPR_MIN_WIDTH | LPR_FUNC_OPT_ENABLE;

        std::string strBpNetDataPath = "/mnt/unused/summary/media_processor/cfg";
        std::string strPath = "/mnt/unused/summary/media_processor/log/";
        strncpy(m_stLprInterface.acLprLogPath, strPath.c_str(), sizeof(m_stLprInterface.acLprLogPath));
        strncpy(m_stLprInterface.acLprBpNetPath, strBpNetDataPath.c_str(), sizeof(m_stLprInterface.acLprBpNetPath));

        int32_t lRet = H3C_LPR_Interface(LPR_CREATE, &m_stLprInterface, NULL, &m_hLprHandle, NULL, NULL, NULL);

        if (LPR_OK != lRet) {
            cout << "LPR_Interface_Fn Create Failed! lRet = " << lRet;
            throw std::runtime_error("create lpr handle error");
        }
    }

    void lprWork(shared_ptr<Yuv420> &picInfo, H3C_REC_RESULT_S *pResult) {
        if (NULL == m_hLprHandle) {
            lprInit();
        }

        /*对DPS的车牌识别进行初始化设和宽高更改进行再次初始化*/
        // 重新创建资源
        m_stLprInterface.iImgHgt = picInfo->getHeight();
        m_stLprInterface.iImgWid = picInfo->getWidth();
        m_stLprInterface.iStride = picInfo->getWidth();

        m_stLprInterface.iROICnt = 1;

        // 区域裁减1/6
        m_stLprInterface.pstROISet->iXTopLeft = 0;
        m_stLprInterface.pstROISet->iYTopLeft = m_stLprInterface.iImgHgt / 6 - 1;

        m_stLprInterface.pstROISet->iXTopRight = m_stLprInterface.iImgWid - 1;
        m_stLprInterface.pstROISet->iYTopRight = m_stLprInterface.iImgHgt / 6 - 1;

        m_stLprInterface.pstROISet->iXBottmLeft = 0;
        m_stLprInterface.pstROISet->iYBottmLeft = m_stLprInterface.iImgHgt - 1;

        m_stLprInterface.pstROISet->iXBottmRight = m_stLprInterface.iImgWid - 1;
        m_stLprInterface.pstROISet->iYBottmRight = m_stLprInterface.iImgHgt - 1;

        m_stLprInterface.uiEnableFlag |= LPR_MIN_WIDTH;
        m_stLprInterface.uiEnableFlag |= LPR_FUNC_OPT_ENABLE;

        m_stLprInterface.iDefaultC = 3;
        m_stLprInterface.iDefaultL = 0;
        m_stLprInterface.usMaxWidth = 160;
        m_stLprInterface.usMinWidth = 100;
        m_stLprInterface.fGammaAdj = 1.0;

        int32_t lRet = H3C_LPR_Interface(LPR_RESET, &m_stLprInterface, NULL, m_hLprHandle, NULL, NULL, NULL);

        if (LPR_OK != lRet) {
            cout << "LPR_Interface_Fn Reset Failed! lRet = " << lRet;
            throw std::runtime_error("lpr reset error");
        }

        LPR_ROI_SET_S stRoiSetEx = { 0 };
        stRoiSetEx.stPlateSet.uiOclNums = 0;
        stRoiSetEx.eColorMode = LPR_WHITE_LIGHT;
        stRoiSetEx.uiEigenMode = 0;
        stRoiSetEx.iROINum = 1;
        stRoiSetEx.aiROIIndex[0] = 0;  // 就一个感兴趣区域
        stRoiSetEx.iLPRNumInROI[0] = 1;  // 每个感兴趣区域只能输出一张车牌，不支持多张，所以结果也只能是一张
        stRoiSetEx.pucImage = picInfo->getY();
        stRoiSetEx.pucImageU = picInfo->getU();
        stRoiSetEx.pucImageV = picInfo->getV();

        // Lpr工作函数
        lRet = H3C_LPR_Interface(LPR_WORK, NULL, &stRoiSetEx, m_hLprHandle, pResult, NULL, NULL);
        
        if (LPR_OK != lRet) {
            cout << "LPR_Interface_Fn Work Failed! lRet = " << lRet;
            throw std::runtime_error("lpr work error!");
        }
    }
};


#endif  // MEDIA_ALGORITHM_H_
