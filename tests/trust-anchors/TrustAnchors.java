// SPDX-License-Identifier: GPL-3.0-only
import java.io.*;
import java.security.*;
import java.security.cert.*;
import javax.net.ssl.*;

public class TrustAnchors {
    static int checks;
    static X509Certificate cert(String path) throws Exception {
        try (InputStream in = new FileInputStream(path)) {
            return (X509Certificate)CertificateFactory.getInstance("X.509").generateCertificate(in);
        }
    }
    static X509TrustManager trust(X509Certificate anchor, String provider) throws Exception {
        KeyStore store = KeyStore.getInstance(KeyStore.getDefaultType()); store.load(null,null);
        if (anchor != null) store.setCertificateEntry("anchor",anchor);
        TrustManagerFactory factory = provider.equals("platform") ? TrustManagerFactory.getInstance("PKIX") : TrustManagerFactory.getInstance("X509",provider);
        factory.init(store);
        return (X509TrustManager)factory.getTrustManagers()[0];
    }
    static void verify(X509TrustManager manager, X509Certificate[] chain, boolean accepted, String label) throws Exception {
        boolean actual;
        try { manager.checkServerTrusted(chain, chain[0].getPublicKey().getAlgorithm()); actual=true; }
        catch (CertificateException | RuntimeException e) { actual=false; }
        if (actual != accepted) throw new AssertionError(label+": expected="+accepted+" actual="+actual);
        checks++; System.out.println("PASS "+label);
    }
    public static void main(String[] args) throws Exception {
        String provider=args[1];
        if (!provider.equals("platform")) Security.addProvider((Provider)Class.forName("com.wolfssl.provider.jsse.WolfSSLProvider").newInstance());
        X509Certificate rsa=cert(args[0]+"/rsa.pem"), ec=cert(args[0]+"/ec.pem"), other=cert(args[0]+"/other.pem"), ca=cert(args[0]+"/ca.pem"), leaf=cert(args[0]+"/leaf.pem");
        if (rsa.getBasicConstraints()!=-1 || ec.getBasicConstraints()!=-1) throw new AssertionError("fixtures must be non-CA");
        verify(trust(rsa,provider), new X509Certificate[]{rsa},true,"explicit RSA leaf anchor");
        verify(trust(ec,provider), new X509Certificate[]{ec},true,"explicit EC leaf anchor");
        verify(trust(rsa,provider),new X509Certificate[]{other},false,"untrusted same-subject different key");
        verify(trust(null,provider),new X509Certificate[]{rsa},false,"empty trust store");
        verify(trust(ca,provider),new X509Certificate[]{leaf},true,"valid CA chain");
        verify(trust(rsa,provider),new X509Certificate[]{leaf,ca},false,"untrusted CA supplied by peer");
        byte[] damaged=leaf.getEncoded(); damaged[damaged.length-1]^=1;
        X509Certificate forged=(X509Certificate)CertificateFactory.getInstance("X.509").generateCertificate(new ByteArrayInputStream(damaged));
        verify(trust(ca,provider),new X509Certificate[]{forged},false,"invalid signature");
        if(trust(rsa,provider).getAcceptedIssuers().length!=1) throw new AssertionError("anchor not exposed");
        System.out.println("TRUST_ANCHORS_PASS checks="+(checks+1)); System.out.flush();
        Runtime.getRuntime().halt(0);
    }
}
